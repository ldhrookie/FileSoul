param(
    [switch]$SkipBuild,
    [switch]$LocalOnly,
    [string]$Model
)

$ErrorActionPreference = "Stop"
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)

[Console]::InputEncoding = $utf8NoBom
[Console]::OutputEncoding = $utf8NoBom
$global:OutputEncoding = $utf8NoBom
chcp.com 65001 | Out-Null

$hadApiKey = Test-Path Env:OPENAI_API_KEY
$previousApiKey = $env:OPENAI_API_KEY
$hadModel = Test-Path Env:FILESOUL_LLM_MODEL
$previousModel = $env:FILESOUL_LLM_MODEL

Push-Location $PSScriptRoot
try {
    if ($LocalOnly) {
        Remove-Item Env:OPENAI_API_KEY -ErrorAction SilentlyContinue
    }

    if ($Model) {
        $env:FILESOUL_LLM_MODEL = $Model
    }

    if (-not $LocalOnly -and -not $env:OPENAI_API_KEY) {
        Write-Host "OPENAI_API_KEY is not set."
        Write-Host "Enter an API key for LLM dialogue. It is used only for this run and is not saved."
        $secureKey = Read-Host "API key (press Enter to use local dialogue)" -AsSecureString

        if ($secureKey.Length -gt 0) {
            $keyPointer = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secureKey)
            try {
                $env:OPENAI_API_KEY = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($keyPointer)
            }
            finally {
                [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($keyPointer)
            }
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
        $env:OPENAI_API_KEY = $previousApiKey
    }
    else {
        Remove-Item Env:OPENAI_API_KEY -ErrorAction SilentlyContinue
    }

    if ($hadModel) {
        $env:FILESOUL_LLM_MODEL = $previousModel
    }
    else {
        Remove-Item Env:FILESOUL_LLM_MODEL -ErrorAction SilentlyContinue
    }

    Pop-Location
}
