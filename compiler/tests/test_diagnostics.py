from openvn.diagnostics import Diagnostic


def test_diagnostic_dict():
    d = Diagnostic("error", "OVN001", "Example", "main.ink", 7, 1)
    assert d.to_dict()["code"] == "OVN001"
