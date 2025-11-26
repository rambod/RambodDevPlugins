# LOD Profile Tool

LOD Profile Tool is an Unreal Engine editor-only plugin that standardizes static mesh LOD generation at scale. It provides a configurable profile (LOD count, screen sizes, reduction percentages, overwrite behavior) and applies it to large selections or entire folders with one click.

## Features
- Editor-only module (UE 5.4–5.7)
- Nomad tab: **Window → LOD Profile Tool**
- Content Browser action: **LOD Tools → Apply LOD Profile** on static meshes
- Profile controls: LOD count, screen sizes, triangle percentages, enable/disable reduction, override existing LODs
- Actions: apply to selection, validate profile
- Project settings under **Edit → Project Settings → Plugins → LOD Profile Tool**

## Project Settings
Values are stored in `Config/DefaultLODProfileTool.ini` and exposed in Project Settings:
- Default number of LODs
- Default screen sizes (0–1)
- Default reduction percentages (0–100)
- Warn on overwrite
- Auto-save modified assets

## Usage
1. Configure defaults in Project Settings → Plugins → LOD Profile Tool.
2. Open the tab (Window → LOD Profile Tool) to tweak a profile.
3. Click **Apply to Selection** (Content Browser assets).
4. Use **Validate Profile** to check inputs.

## Packaging Notes
- Directory layout follows Fab requirements.
- No game/runtime code; module type is `Editor`.
- No content assets included; code-only plugin.
