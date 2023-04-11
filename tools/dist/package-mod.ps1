param ($ReleaseBin, $ProjectName = "TweakXL")

$StageDir = "build/package"
$DistDir = "build/dist"

& $($PSScriptRoot + "\steps\compose-plugin-red4ext.ps1") -StageDir ${StageDir} -ProjectName ${ProjectName} -ReleaseBin ${ReleaseBin}
& $($PSScriptRoot + "\steps\compose-redscripts.ps1") -StageDir ${StageDir} -ProjectName ${ProjectName}
& $($PSScriptRoot + "\steps\make-tweaks-dir.ps1") -StageDir ${StageDir}
& $($PSScriptRoot + "\steps\create-zip-from-stage.ps1") -StageDir ${StageDir} -ProjectName ${ProjectName} -DistDir ${DistDir}

Remove-Item -Recurse ${StageDir}
