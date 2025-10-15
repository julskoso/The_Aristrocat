using UnrealBuildTool;
using System.Collections.Generic;

public class The_AristrocatEditorTarget : TargetRules
{
    public The_AristrocatEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("The_Aristrocat");
    }
}
