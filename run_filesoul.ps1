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
        Write-Host "OPENAI_API_KEY가 없습니다. LLM 대사를 사용하려면 API 키를 입력하세요."
        Write-Host "입력한 키는 저장되지 않고 이번 실행에만 사용됩니다."
        $secureKey = Read-Host "API 키 (빈 입력은 로컬 대사 사용)" -AsSecureString

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
        $sources = @(Get-ChildItem -LiteralPath $PSScriptRoot -Filter "*.c" |
            ForEach-Object { $_.FullName })

        & gcc -Wall -Wextra @sources -o "$PSScriptRoot\filesoul.exe"
        if ($LASTEXITCODE -ne 0) {
            throw "FileSoul 빌드에 실패했습니다."
        }
    }

    & "$PSScriptRoot\filesoul.exe"
    if ($LASTEXITCODE -ne 0) {
        throw "FileSoul 실행이 실패했습니다. 종료 코드: $LASTEXITCODE"
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
