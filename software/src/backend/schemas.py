from pydantic import BaseModel
from typing import Optional

class CustomerCreate(BaseModel):
    username: str
    password: str
    face_id: Optional[str] = None


class CourierCreate(BaseModel):
    username: str
    password: str


class LockerCreate(BaseModel):
    locker_number: str

class LockerStatusUpdate(BaseModel):
    status: str



class OrderCreate(BaseModel):
    tracking_code: str
    courier_id: Optional[int] = None
    customer_id: Optional[int] = None
    customer_username: Optional[str] = None
    locker_id: Optional[int] = None

class VerifyFaceAndPickup(BaseModel):
    tracking_code: str
    customer_id: Optional[int] = None
    customer_username: Optional[str] = None
    face_id: Optional[str] = None