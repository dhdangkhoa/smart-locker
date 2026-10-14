from fastapi import FastAPI, Depends, HTTPException
from sqlalchemy.orm import Session
from database import engine, get_db, SessionLocal
import models
import schemas

models.Base.metadata.create_all(bind=engine)

app = FastAPI(title="Smart Locker")


#compartment management
@app.post("/lockers/", summary="Compartment Management")
def create_compartment(locker: schemas.LockerCreate, db: Session = Depends(get_db)):

    existing = db.query(models.Locker).filter(models.Locker.locker_number == locker.locker_number).first()

    if existing: 
        raise HTTPException(status_code=400, detail="This compartment already exists!")

    db_locker = models.Locker(
        locker_number = locker.locker_number
    )
    db.add(db_locker)
    db.commit()
    db.refresh(db_locker)
    return {"message": "Installed Successfully", "locker_number": db_locker.locker_number}


# customer management 
@app.post("/customers/", summary="Customer Account Registration")
def create_customer(customer: schemas.CustomerCreate, db: Session = Depends(get_db)):
    
    existing = db.query(models.Customer).filter(models.Customer.username == customer.username).first()
    
    if existing:
        raise HTTPException(status_code=400, detail="This username already exists!")

    db_customer = models.Customer(
        username=customer.username,
        password=customer.password,
        face_id=customer.face_id
    )
    db.add(db_customer)
    db.commit()
    db.refresh(db_customer)
    return {"message": "Created Successfully!", "customer_id": db_customer.id}

# courier management
@app.post("/couriers/", summary="Courier Account Registration")
def create_courier(courier: schemas.CourierCreate, db: Session = Depends(get_db)):
    
    existing = db.query(models.Courier).filter(models.Courier.username == courier.username).first()
    
    if existing:
        raise HTTPException(status_code=400, detail="This username is already exists!")

    db_courier = models.Courier(
        username = courier.username,
        password = courier.password
    )
    db.add(db_courier)
    db.commit()
    db.refresh(db_courier)

    return {"message": "Created Successfully!", "courier_id": db_courier.id}


# package delivery
@app.post("/orders/store", summary="Courier Deposits Package into Locker (Create Order & Update Locker Status)")
def store_order(order:schemas.OrderCreate, db: Session = Depends(get_db)):

    locker = db.query(models.Locker).filter(models.Locker.id == order.locker_id).first()
    if not locker:
        raise HTTPException(status_code=404, detail="Not Found")
    if locker.status != "available":
        raise HTTPException(status_code=400, detail="This compartment is currently occupied or under maintenance!")

    existing_order = db.query(models.Order).filter(models.Order.tracking_code == order.tracking_code).first()
    if existing_order:
        raise HTTPException(status_code=400, detail="This order code already exists in the system!")


    new_order = models.Order(
        tracking_code = order.tracking_code,
        courier_id = order.courier_id,
        customer_id = order.customer_id,
        locker_id = order.locker_id,
        status = "waiting"
    )
    db.add(new_order)

    locker.status = "occupied"

    db.commit()
    return {
        "message": "Package deposited successfully! The locker is now locked.",
        "locker_number": locker.locker_number,
        "tracking_code": new_order.tracking_code
    }


# package pickup
@app.post("/orders/pickup", summary="Customer Pickup (Order Code Verification + Face Scan)")
def pickup_order(data: schemas.VerifyFaceAndPickup, db: Session = Depends(get_db)):
    order = db.query(models.Order).filter(
        models.Order.tracking_code == data.tracking_code,
        models.Order.status == "waiting"
    ).first()

    if not order:
        raise HTTPException(status_code=404, detail="Invalid order code or the package has already been picked up!")

    customer = db.query(models.Customer).filter(models.Customer.id == order.customer_id).first()
    if not customer:
        raise HTTPException(status_code=404, detail="Recipient information for this order could not be found!")

    if not customer.face_id or customer.face_id != data.face_id:
        raise HTTPException(status_code=401, detail="Face verification failed! The face does not match the registered recipient of this order.")

    order.status = "received"

    locker = db.query(models.Locker).filter(models.Locker.id == order.locker_id).first()

    if locker:
        locker.status = "available"

    db.commit()

    return {
        "message": "Face and order code verified successfully! Unlocking locker number:",
        "locker_number": locker.locker_number if locker else None,
        "customer_name": customer.username
    }