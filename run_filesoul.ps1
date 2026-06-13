param(
    [switch]$SkipBuild,
    [switch]$LocalOnly,
    [switch]$Popup,
    [string]$Model
)

$ErrorActionPreference = "Stop"
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)

[Console]::InputEncoding = $utf8NoBom
[Console]::OutputEncoding = $utf8NoBom
$global:OutputEncoding = $utf8NoBom
chcp.com 65001 | Out-Null

$hadApiKey = Test-Path Env:GROQ_API_KEY
$previousApiKey = $env:GROQ_API_KEY
$hadModel = Test-Path Env:FILESOUL_LLM_MODEL
$previousModel = $env:FILESOUL_LLM_MODEL
$hadVerificationFailed = Test-Path Env:FILESOUL_LLM_VERIFICATION_FAILED
$previousVerificationFailed = $env:FILESOUL_LLM_VERIFICATION_FAILED
$hadTerminalDialogue = Test-Path Env:FILESOUL_TERMINAL_DIALOGUE
$previousTerminalDialogue = $env:FILESOUL_TERMINAL_DIALOGUE

function Test-GroqDialogueRequest {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ApiKey,
        [Parameter(Mandatory = $true)]
        [string]$ModelName
    )

    $body = @{
        model = $ModelName
        messages = @(
            @{
                role = "user"
                content = "Return exactly FILESOUL_LLM_OK for FileSoul startup verification."
            }
        )
        max_tokens = 12
        temperature = 0
    } | ConvertTo-Json -Depth 5

    Write-Host "LLM verification: sending a real Groq request now..."
    try {
        $response = Invoke-RestMethod `
            -Method Post `
            -Uri "https://api.groq.com/openai/v1/chat/completions" `
            -Headers @{
                Authorization = "Bearer $ApiKey"
                "Content-Type" = "application/json"
            } `
            -Body $body `
            -TimeoutSec 30

        $content = $response.choices[0].message.content
        if ($content -and $content.Trim().Length -gt 0) {
            Write-Host "LLM verification passed: received API dialogue test response: $($content.Trim())"
            return $true
        }

        Write-Host "LLM verification failed: Groq returned no readable dialogue text."
        return $false
    }
    catch {
        $statusCode = $null
        $errorText = $_.Exception.Message

        if ($_.Exception.Response -ne $null) {
            $statusCode = [int]$_.Exception.Response.StatusCode
            try {
                $reader = New-Object System.IO.StreamReader($_.Exception.Response.GetResponseStream())
                $responseText = $reader.ReadToEnd()
                if ($responseText) {
                    $errorText = $responseText
                }
            }
            catch {
            }
        }

        if ($statusCode -ne $null) {
            Write-Host "LLM verification failed: HTTP $statusCode"
        }
        else {
            Write-Host "LLM verification failed: request could not complete."
        }
        Write-Host ($errorText | Out-String).Trim()
        return $false
    }
}

Push-Location $PSScriptRoot
try {
    if ($LocalOnly) {
        Remove-Item Env:GROQ_API_KEY -ErrorAction SilentlyContinue
    }

    if ($Model) {
        $env:FILESOUL_LLM_MODEL = $Model
    }

    if (-not $Popup) {
        $env:FILESOUL_TERMINAL_DIALOGUE = "1"
        Write-Host "Terminal choice mode is enabled. Use -Popup to show Windows popups."
    }

    if (-not $LocalOnly -and -not $env:GROQ_API_KEY) {
        Write-Host "GROQ_API_KEY is not set."
        Write-Host "Enter a Groq API key for LLM dialogue. It is used only for this run and is not saved."
        $secureKey = Read-Host "API key (press Enter to use local dialogue)" -AsSecureString

        if ($secureKey.Length -gt 0) {
            $keyPointer = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secureKey)
            try {
                $env:GROQ_API_KEY = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($keyPointer)
            }
            finally {
                [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($keyPointer)
            }
        }
    }

    if (-not $LocalOnly -and $env:GROQ_API_KEY) {
        $modelName = if ($env:FILESOUL_LLM_MODEL) { $env:FILESOUL_LLM_MODEL } else { "llama-3.1-8b-instant" }
        if (Test-GroqDialogueRequest -ApiKey $env:GROQ_API_KEY -ModelName $modelName) {
            Remove-Item Env:FILESOUL_LLM_VERIFICATION_FAILED -ErrorAction SilentlyContinue
        }
        else {
            $env:FILESOUL_LLM_VERIFICATION_FAILED = "startup API request failed"
            Remove-Item Env:GROQ_API_KEY -ErrorAction SilentlyContinue
            Write-Host "LLM dialogue disabled for this run. Local dialogue will be shown only as the final fallback."
        }
    }

    if (-not $SkipBuild) {
        $executablePath = "$PSScriptRoot\filesoul.exe"
        $runningFileSoul = @(Get-Process -Name "filesoul" -ErrorAction SilentlyContinue |
            Where-Object { $_.Path -eq $executablePath })

        if ($runningFileSoul.Count -gt 0) {
            Write-Host "FileSoul is already running."
            Write-Host "Close every FileSoul popup/window, then run this launcher again."
            throw "Cannot rebuild while filesoul.exe is running."
        }
        else {
            & gcc -Wall -Wextra "*.c" -o "filesoul.exe"
            if ($LASTEXITCODE -ne 0) {
                throw "FileSoul build failed. Close FileSoul if it is running, then try again."
            }
        }
    }

    & "$PSScriptRoot\filesoul.exe"
    if ($LASTEXITCODE -ne 0) {
        throw "FileSoul failed with exit code: $LASTEXITCODE"
    }
}
finally {
    if ($hadApiKey) {
        $env:GROQ_API_KEY = $previousApiKey
    }
    else {
        Remove-Item Env:GROQ_API_KEY -ErrorAction SilentlyContinue
    }

    if ($hadModel) {
        $env:FILESOUL_LLM_MODEL = $previousModel
    }
    else {
        Remove-Item Env:FILESOUL_LLM_MODEL -ErrorAction SilentlyContinue
    }

    if ($hadVerificationFailed) {
        $env:FILESOUL_LLM_VERIFICATION_FAILED = $previousVerificationFailed
    }
    else {
        Remove-Item Env:FILESOUL_LLM_VERIFICATION_FAILED -ErrorAction SilentlyContinue
    }

    if ($hadTerminalDialogue) {
        $env:FILESOUL_TERMINAL_DIALOGUE = $previousTerminalDialogue
    }
    else {
        Remove-Item Env:FILESOUL_TERMINAL_DIALOGUE -ErrorAction SilentlyContinue
    }

    Pop-Location
}
