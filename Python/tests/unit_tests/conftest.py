import pytest # might not need this, but can't hurt?

def pytest_configure(config):
    """ configure pytest"""
    # ignore unknown marker warning
    config.addinivalue_line("filterwarnings", "ignore::pytest.PytestUnknownMarkWarning")
    # register some markers anyway
    config.addinivalue_line("markers", "m_validated: tests validated by Stephen Arsenault as correctly implemented")
    config.addinivalue_line("markers", "m_bug: tests which identify a bug")
    config.addinivalue_line("markers", "m_in_development: tests currently being worked on but not yet fully finalized and validated")
    config.addinivalue_line("markers", 'm_missing_validation: known to fail due to missing validation, use -m "not m_missing_validation" to skip these')
    config.addinivalue_line("markers", 'm_missing_feature: known to fail due to missing feature, use -m "not m_missing_feature" to skip these')
