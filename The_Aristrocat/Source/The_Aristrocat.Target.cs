using UnrealBuildTool;
using System.Collections.Generic;

public class The_AristrocatTarget : TargetRules
{
    public The_AristrocatTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("The_Aristrocat");
    }
}
