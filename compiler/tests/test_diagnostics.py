from openvn.diagnostics import Diagnostic


def test_diagnostic_dict() -> None:
    diagnostic = Diagnostic("error", "OVN001", "Example", "main.ink", 7, 1)
    assert diagnostic.to_dict()["code"] == "OVN001"


def test_diagnostic_text_with_location() -> None:
    diagnostic = Diagnostic("error", "OVN001", "bad syntax", "main.ink", 7, 1)
    assert diagnostic.format_text() == "main.ink:7:1: error OVN001: bad syntax"


def test_diagnostic_text_without_location() -> None:
    diagnostic = Diagnostic("warning", "OVN004", "unreachable node")
    assert diagnostic.format_text() == "warning OVN004: unreachable node"
