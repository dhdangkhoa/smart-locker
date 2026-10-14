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



class OrderCreate(BaseModel):
    tracking_code: str
    courier_id: int
    customer_id: int
    locker_id: int

class VerifyFaceAndPickup(BaseModel):
    tracking_code: str
    face_id: str