"""Authentication utilities: password hashing and JWT handling."""

import hashlib
from datetime import datetime, timedelta, timezone
from typing import Optional, Tuple

import bcrypt
import jwt
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.config import get_settings
from app.models import User, RefreshToken

settings = get_settings()


def hash_sha256(password: str) -> str:
    """Hash password using SHA256 (matches client-side hashing)."""
    return hashlib.sha256(password.encode()).hexdigest()


def hash_password(password: str) -> str:
    """Hash password using bcrypt for storage.

    Client sends SHA256 hashed password, we bcrypt that hash.
    """
    # bcrypt accepts bytes, max 72 bytes
    salt = bcrypt.gensalt(rounds=12)
    return bcrypt.hashpw(password.encode(), salt).decode()


def verify_password(plain_password: str, hashed_password: str) -> bool:
    """Verify password against bcrypt hash."""
    return bcrypt.checkpw(plain_password.encode(), hashed_password.encode())


def create_jwt_token(
    user_id: str,
    token_type: str = "access",
    expires_delta: Optional[timedelta] = None,
    account: Optional[str] = None
) -> Tuple[str, datetime]:
    """Create a JWT token.

    Returns:
        Tuple of (token, expiration_datetime)
    """
    if expires_delta is None:
        if token_type == "access":
            expires_delta = timedelta(minutes=settings.JWT_ACCESS_TOKEN_EXPIRE_MINUTES)
        else:
            expires_delta = timedelta(days=settings.JWT_REFRESH_TOKEN_EXPIRE_DAYS)

    expire = datetime.now(timezone.utc) + expires_delta

    payload = {
        "sub": user_id,
        "type": token_type,
        "exp": expire,
        "iat": datetime.now(timezone.utc),
    }

    if account:
        payload["account"] = account

    token = jwt.encode(
        payload,
        settings.JWT_SECRET_KEY,
        algorithm=settings.JWT_ALGORITHM
    )

    return token, expire


def decode_jwt_token(token: str) -> Optional[dict]:
    """Decode and verify a JWT token.

    Returns:
        Payload dict if valid, None if invalid
    """
    try:
        payload = jwt.decode(
            token,
            settings.JWT_SECRET_KEY,
            algorithms=[settings.JWT_ALGORITHM]
        )
        return payload
    except jwt.ExpiredSignatureError:
        return None
    except jwt.InvalidTokenError:
        return None


async def create_refresh_token(
    session: AsyncSession,
    user_id: str
) -> Tuple[str, datetime]:
    """Create a refresh token and store it in database."""
    token, expires_at = create_jwt_token(
        user_id=user_id,
        token_type="refresh"
    )

    # Store token hash in database for revocation support
    token_hash = hashlib.sha256(token.encode()).hexdigest()

    refresh_token = RefreshToken(
        user_id=user_id,
        token_hash=token_hash,
        expires_at=expires_at
    )

    session.add(refresh_token)
    await session.commit()

    return token, expires_at


async def verify_refresh_token(
    session: AsyncSession,
    token: str
) -> Optional[str]:
    """Verify a refresh token and return user_id if valid.

    Returns:
        user_id if valid, None if invalid
    """
    # First decode and verify JWT
    payload = decode_jwt_token(token)
    if not payload or payload.get("type") != "refresh":
        return None

    user_id = payload.get("sub")
    if not user_id:
        return None

    # Check if token is revoked
    token_hash = hashlib.sha256(token.encode()).hexdigest()
    result = await session.execute(
        select(RefreshToken).where(
            RefreshToken.token_hash == token_hash,
            RefreshToken.is_revoked == False,
            RefreshToken.expires_at > datetime.now(timezone.utc)
        )
    )
    db_token = result.scalar_one_or_none()

    if not db_token:
        return None

    return user_id


async def revoke_refresh_token(session: AsyncSession, token: str):
    """Revoke a refresh token."""
    token_hash = hashlib.sha256(token.encode()).hexdigest()
    result = await session.execute(
        select(RefreshToken).where(RefreshToken.token_hash == token_hash)
    )
    db_token = result.scalar_one_or_none()

    if db_token:
        db_token.is_revoked = True
        await session.commit()


async def authenticate_user(
    session: AsyncSession,
    account: str,
    password: str
) -> Optional[User]:
    """Authenticate a user by account and password."""
    result = await session.execute(
        select(User).where(User.account == account, User.is_active == True)
    )
    user = result.scalar_one_or_none()

    if not user:
        return None

    if not verify_password(password, user.password_hash):
        return None

    return user


def user_to_response(user: User) -> dict:
    """Convert User model to response dict."""
    return {
        "userId": user.id,
        "account": user.account,
        "displayName": user.display_name,
        "phone": user.phone,
        "email": user.email,
        "avatarUrl": user.avatar_url,
    }
