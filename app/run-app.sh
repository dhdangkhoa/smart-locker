#!/bin/bash


echo "Backend FastAPI"
cd backend/
uvicorn main:app --reload &
BACKEND_PID=$! 

sleep 2

echo "Frontend Flutter Web"
cd ../frontend/
flutter run -d web-server --web-port=3000 &
FLUTTER_PID=$! 

trap "echo 'Quit'; kill $BACKEND_PID $FLUTTER_PID; exit" INT

wait