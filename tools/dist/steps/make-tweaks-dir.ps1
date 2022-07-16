param ($StageDir)

$TweaksDir = "${StageDir}/r6/tweaks"

New-Item -ItemType directory -Force -Path ${TweaksDir} | Out-Null
