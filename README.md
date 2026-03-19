# Alchemy Recipe View VR - SKSE

Native Skyrim VR rewrite of the Alchemy Recipe View behavior.

Current scope:
- own recipe-view runtime entirely in SKSE/CommonLib
- detect the alchemy subtype through Scaleform instead of `GetCraftingSubMenu()`
- inject a minimal button shim SWF owned by this project
- build native ingredient registry, player snapshot, and synthetic alchemy rows
- label synthetic rows through the native alchemy row-to-GFx hook
- block crafting natively while recipe mode is enabled

Build:

```powershell
xmake build
```

Deploy assets staged in:
- `deploy/SKSE/Plugins/AlchemyRecipeViewVR.ini`
- `deploy/Interface/AlchemyRecipeViewVR.swf`
- `deploy/Interface/Translations/`

Reference dependencies:
- local `commonlibsse-ng` under `lib/commonlibsse-ng`, or
- shared clone at `../CustomSkillsFrameworkVR/lib/commonlibsse-ng`
