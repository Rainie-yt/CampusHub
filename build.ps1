$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $sources = @('main.cpp') + @(Get-ChildItem -LiteralPath 'src' -Recurse -Filter '*.cpp' | ForEach-Object { $_.FullName })
    & g++ -std=c++11 -Wall -Wextra -finput-charset=UTF-8 -fexec-charset=UTF-8 -static @sources -o campushub.exe
    if ($LASTEXITCODE -ne 0) { throw 'CampusHub build failed.' }
    Write-Output 'Built campushub.exe. Run from the project directory.'
} finally {
    Pop-Location
}
