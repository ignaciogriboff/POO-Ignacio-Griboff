"""
HTTP Basic Auth dependency.
Credentials are read from environment variables BASIC_AUTH_USER / BASIC_AUTH_PASS.
"""
import secrets
from fastapi import Depends, HTTPException, status
from fastapi.security import HTTPBasic, HTTPBasicCredentials
from app.config import get_settings

security = HTTPBasic()


def verify_basic_auth(
    credentials: HTTPBasicCredentials = Depends(security),
) -> str:
    settings = get_settings()
    valid_user = secrets.compare_digest(
        credentials.username.encode(), settings.basic_auth_user.encode()
    )
    valid_pass = secrets.compare_digest(
        credentials.password.encode(), settings.basic_auth_pass.encode()
    )
    if not (valid_user and valid_pass):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid credentials",
            headers={"WWW-Authenticate": "Basic"},
        )
    return credentials.username


def check_ws_auth(u: str, p: str) -> bool:
    """Used for WebSocket authentication via query string."""
    settings = get_settings()
    valid_user = secrets.compare_digest(u.encode(), settings.basic_auth_user.encode())
    valid_pass = secrets.compare_digest(p.encode(), settings.basic_auth_pass.encode())
    return valid_user and valid_pass
