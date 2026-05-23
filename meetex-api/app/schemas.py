"""Pydantic schemas for request/response validation."""

from datetime import datetime
from typing import Optional

from pydantic import BaseModel, Field, EmailStr


# ============ User Schemas ============

class UserBase(BaseModel):
    """Base user schema."""
    display_name: str = Field(..., min_length=1, max_length=100)
    phone: Optional[str] = Field(None, max_length=20)
    email: Optional[EmailStr] = None
    avatar_url: Optional[str] = Field(None, max_length=500)


class UserCreate(UserBase):
    """User creation schema."""
    account: str = Field(
        ...,
        min_length=3,
        max_length=20,
        pattern=r"^[a-zA-Z0-9_]+$",
        description="账号只能包含字母、数字、下划线"
    )
    password: str = Field(..., min_length=8, description="密码最少8位")
    phone: str = Field(..., pattern=r"^1[3-9]\d{9}$", description="手机号必填")
    code: str = Field(..., min_length=4, max_length=6, description="短信验证码")


class UserResponse(BaseModel):
    """User response schema (matches client expectations)."""
    user_id: str
    display_name: str
    phone: Optional[str] = None
    email: Optional[str] = None
    avatar_url: Optional[str] = None

    class Config:
        from_attributes = True


# ============ Auth Schemas ============

class LoginRequest(BaseModel):
    """Account/password login request."""
    account: str = Field(..., min_length=1, max_length=50)
    password: str = Field(..., min_length=1)  # SHA256 hash from client


class PhoneLoginRequest(BaseModel):
    """Phone/SMS login request."""
    phone: str = Field(..., pattern=r"^1[3-9]\d{9}$")
    code: str = Field(..., min_length=4, max_length=6)


class TokenData(BaseModel):
    """Token response data."""
    access_token: str
    refresh_token: str
    expires_in: datetime


class LoginResponse(BaseModel):
    """Login response wrapper (matches client expectations)."""
    code: int = 200
    message: str = "success"
    data: dict


class RefreshRequest(BaseModel):
    """Token refresh request."""
    refresh_token: str


class TokenPayload(BaseModel):
    """JWT token payload."""
    sub: str  # user_id
    account: Optional[str] = None
    exp: Optional[datetime] = None
    type: str = "access"  # access or refresh
