"""Authentication API routes."""

from datetime import datetime, timezone

from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_db
from app.models import User
from app.schemas import LoginRequest, PhoneLoginRequest, RefreshRequest, LoginResponse, UserCreate
from app.auth_utils import (
    authenticate_user,
    create_jwt_token,
    create_refresh_token,
    verify_refresh_token,
    user_to_response,
    hash_password,
)

router = APIRouter(prefix="/v1/auth", tags=["auth"])


@router.post("/register", response_model=LoginResponse)
async def register(
    request: UserCreate,
    session: AsyncSession = Depends(get_db)
):
    """User registration endpoint.

    Client sends SHA256 hashed password. We bcrypt it for storage.
    Verifies SMS code before creating account.
    """
    # Verify SMS code first
    # TODO: Integrate with actual SMS service
    # For now, accept "123456" for development
    if request.code != "123456":
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Invalid verification code"
        )

    # Check if account already exists
    result = await session.execute(
        select(User).where(User.account == request.account)
    )
    if result.scalar_one_or_none():
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail="Account already exists"
        )

    # Check if phone already exists
    result = await session.execute(
        select(User).where(User.phone == request.phone)
    )
    if result.scalar_one_or_none():
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail="Phone number already registered"
        )

    # Create new user with all fields
    user = User(
        account=request.account,
        password_hash=hash_password(request.password),
        display_name=request.display_name,
        phone=request.phone,
        email=request.email,
        avatar_url=request.avatar_url
    )

    session.add(user)
    await session.commit()
    await session.refresh(user)

    # Create tokens
    access_token, _ = create_jwt_token(
        user_id=user.id,
        token_type="access",
        account=user.account
    )
    refresh_token, expires_at = await create_refresh_token(session, user.id)

    return LoginResponse(
        code=200,
        message="success",
        data={
            "accessToken": access_token,
            "refreshToken": refresh_token,
            "expiresIn": expires_at.isoformat(),
            "user": user_to_response(user)
        }
    )


@router.post("/login", response_model=LoginResponse)
async def login(
    request: LoginRequest,
    session: AsyncSession = Depends(get_db)
):
    """Account/password login endpoint.

    Client sends SHA256 hashed password. We verify against bcrypt hash.
    """
    user = await authenticate_user(session, request.account, request.password)

    if not user:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid account or password"
        )

    # Create tokens
    access_token, _ = create_jwt_token(
        user_id=user.id,
        token_type="access",
        account=user.account
    )
    refresh_token, expires_at = await create_refresh_token(session, user.id)

    return LoginResponse(
        code=200,
        message="success",
        data={
            "accessToken": access_token,
            "refreshToken": refresh_token,
            "expiresIn": expires_at.isoformat(),
            "user": user_to_response(user)
        }
    )


@router.post("/login/phone", response_model=LoginResponse)
async def login_phone(
    request: PhoneLoginRequest,
    session: AsyncSession = Depends(get_db)
):
    """Phone/SMS code login endpoint.

    For development, accepts any code "123456".
    In production, should verify against SMS service.
    """
    # TODO: Integrate with SMS service
    # For now, accept any 6-digit code for development
    if request.code != "123456":
        # Check if user exists with this phone
        result = await session.execute(
            select(User).where(User.phone == request.phone, User.is_active == True)
        )
        user = result.scalar_one_or_none()

        if not user:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Invalid phone or code"
            )
    else:
        # Find or create user
        result = await session.execute(
            select(User).where(User.phone == request.phone)
        )
        user = result.scalar_one_or_none()

        if not user:
            # Create new user
            user = User(
                account=request.phone,
                password_hash=hash_password(request.phone),  # Placeholder
                display_name=f"User_{request.phone[-4:]}",
                phone=request.phone
            )
            session.add(user)
            await session.commit()
            await session.refresh(user)

    # Create tokens
    access_token, _ = create_jwt_token(
        user_id=user.id,
        token_type="access",
        account=user.account
    )
    refresh_token, expires_at = await create_refresh_token(session, user.id)

    return LoginResponse(
        code=200,
        message="success",
        data={
            "accessToken": access_token,
            "refreshToken": refresh_token,
            "expiresIn": expires_at.isoformat(),
            "user": user_to_response(user)
        }
    )


@router.post("/refresh", response_model=LoginResponse)
async def refresh_token(
    request: RefreshRequest,
    session: AsyncSession = Depends(get_db)
):
    """Token refresh endpoint."""
    user_id = await verify_refresh_token(session, request.refresh_token)

    if not user_id:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid or expired refresh token"
        )

    # Get user
    result = await session.execute(
        select(User).where(User.id == user_id, User.is_active == True)
    )
    user = result.scalar_one_or_none()

    if not user:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="User not found or inactive"
        )

    # Create new tokens
    access_token, _ = create_jwt_token(
        user_id=user.id,
        token_type="access",
        account=user.account
    )
    refresh_token, expires_at = await create_refresh_token(session, user.id)

    return LoginResponse(
        code=200,
        message="success",
        data={
            "accessToken": access_token,
            "refreshToken": refresh_token,
            "expiresIn": expires_at.isoformat(),
            "user": user_to_response(user)
        }
    )
