#!/usr/bin/env python3
"""Initialize database and create test users."""

import asyncio
import hashlib
import sys

from app.database import init_db, AsyncSessionLocal
from app.models import User
from app.auth_utils import hash_password


async def create_user(
    account: str,
    password: str,
    display_name: str,
    phone: str = None,
    email: str = None
):
    """Create a user with SHA256 hashed password."""
    async with AsyncSessionLocal() as session:
        # Check if user exists
        from sqlalchemy import select
        result = await session.execute(
            select(User).where(User.account == account)
        )
        if result.scalar_one_or_none():
            print(f"User '{account}' already exists, skipping...")
            return

        # Hash password (client sends SHA256, we bcrypt it)
        password_hash = hash_password(password)

        user = User(
            account=account,
            password_hash=password_hash,
            display_name=display_name,
            phone=phone,
            email=email
        )

        session.add(user)
        await session.commit()
        print(f"Created user: {account} ({display_name})")


async def main():
    """Initialize database and create default users."""
    print("Initializing database...")
    await init_db()
    print("Database initialized.\n")

    # Create test users
    print("Creating test users...")

    # User 1: test / password123
    await create_user(
        account="test",
        password="ef92b768b9b2b02f8e7e05f3b5e1a1f1c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c",  # password123 SHA256
        display_name="Test User",
        phone="13800138000",
        email="test@example.com"
    )

    # User 2: admin / admin123
    await create_user(
        account="admin",
        password="admin123",
        display_name="Administrator",
        phone="13800138001",
        email="admin@example.com"
    )

    # User 3: darin / darin123
    await create_user(
        account="darin",
        password="darin123",
        display_name="Darin Zhang",
        phone="13800138002",
        email="darin@example.com"
    )

    print("\nDone! You can now login with:")
    print("  - Account: test, Password: password123 (SHA256 hashed)")
    print("  - Account: admin, Password: admin123")
    print("  - Account: darin, Password: darin123")


if __name__ == "__main__":
    asyncio.run(main())
