#!/bin/bash

# MeetEx API Server Startup Script
# Usage: ./start.sh [dev|prod]

set -e

MODE=${1:-dev}
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check if virtual environment exists, create if not
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
fi

# Activate virtual environment
source venv/bin/activate

# Install dependencies
echo "Installing dependencies..."
pip install -q -r requirements.txt

if [ "$MODE" == "dev" ]; then
    echo "Starting development server..."
    echo "API documentation will be available at:"
    echo "  - Swagger UI: http://localhost:8000/docs"
    echo "  - ReDoc:      http://localhost:8000/redoc"
    echo ""
    uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
elif [ "$MODE" == "prod" ]; then
    echo "Starting production server with Gunicorn..."
    # Install gunicorn if not already installed
    pip install -q gunicorn
    gunicorn -w 4 -k uvicorn.workers.UvicornWorker \
        --bind 0.0.0.0:8000 \
        --access-logfile - \
        --error-logfile - \
        --access-logformat '%(h)s %(l)s %(u)s %(t)s "%(r)s" %(s)s %(b)s "%(f)s" "%(a)s" %(D)s' \
        app.main:app
else
    echo "Usage: ./start.sh [dev|prod]"
    exit 1
fi
