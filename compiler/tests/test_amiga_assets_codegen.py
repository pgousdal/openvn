from pathlib import Path

from openvn.assets import load_asset_manifest
from openvn.backends.amiga.assets_codegen import render_assets_source

REPOSITORY = Path(__file__).parents[2]
PROJECT = REPOSITORY / "examples" / "the-last-lighthouse"


def test_render_asset_table() -> None:
    manifest = load_asset_manifest(PROJECT)
    packaged_paths = {
        source: Path("assets") / source.relative_to(manifest.root)
        for source in manifest.all_files()
    }

    source = render_assets_source(manifest, packaged_paths)

    assert "OPENVN_GENERATED_ASSETS" in source
    assert '{"lighthouse_storm",' in source
    assert '{"erik.neutral",' in source
    assert '{"storm",' in source
    assert '{"thunder",' in source
