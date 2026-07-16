from pathlib import Path

from openvn.schema_validation import validate_document

SCHEMA = Path(__file__).parents[2] / "schemas" / "openvn-story-0.3.schema.json"


def test_valid_document_passes_schema() -> None:
    document = {
        "format": "openvn-story",
        "version": "0.3",
        "entry": "start",
        "symbols": {"start": "start"},
        "nodes": [{"id": "start", "type": "end"}],
    }
    assert validate_document(document, schema_path=SCHEMA) == []


def test_invalid_document_returns_ovn005() -> None:
    document = {
        "format": "openvn-story",
        "version": "0.3",
        "entry": "start",
        "symbols": {},
        "nodes": [],
    }
    diagnostics = validate_document(document, schema_path=SCHEMA)
    assert diagnostics
    assert all(item.code == "OVN005" for item in diagnostics)
