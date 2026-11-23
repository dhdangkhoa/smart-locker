from fastapi import FastAPI, Depends, HTTPException
from sqlalchemy.orm import Session
from database import engine, get_db, SessionLocal
import models
import schemas

from fastapi.middleware.cors import CORSMiddleware

models.Base.metadata.create_all(bind=engine)

app = FastAPI(title="Smart Locker")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], 
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

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


@app.get("/lockers/", summary="List all compartments and their statuses")
def get_lockers(db: Session = Depends(get_db)):
    lockers = db.query(models.Locker).order_by(models.Locker.id).all()
    results = []
    for locker in lockers:
        order = db.query(models.Order).filter(
            models.Order.locker_id == locker.id,
            models.Order.status == "waiting"
        ).first()

        recipient_username = None
        tracking_code = None
        if order:
            recipient = db.query(models.Customer).filter(models.Customer.id == order.customer_id).first()
            recipient_username = recipient.username if recipient else str(order.customer_id)
            tracking_code = order.tracking_code

        results.append({
            "id": str(locker.id),
            "box_number": locker.locker_number,
            "status": locker.status,
            "tracking_code": tracking_code,
            "recipient": recipient_username
        })
    return results


# admin updates compartment status in database (e.g. maintenance / available)
@app.patch("/lockers/{locker_id_or_number}/status", summary="Admin updates locker status in database")
@app.put("/lockers/{locker_id_or_number}/status", summary="Admin updates locker status in database")
def update_locker_status(locker_id_or_number: str, data: schemas.LockerStatusUpdate, db: Session = Depends(get_db)):
    locker = None
    if locker_id_or_number.isdigit():
        locker = db.query(models.Locker).filter(models.Locker.id == int(locker_id_or_number)).first()
    if not locker:
        locker = db.query(models.Locker).filter(models.Locker.locker_number.ilike(locker_id_or_number.strip())).first()

    if not locker:
        raise HTTPException(status_code=404, detail="Compartment not found!")

    target_status = data.status.lower().strip()
    if target_status not in ["available", "maintenance", "occupied"]:
        raise HTTPException(status_code=400, detail="Invalid status! Must be available, maintenance, or occupied.")

    locker.status = target_status
    db.commit()
    db.refresh(locker)

    return {
        "message": f"Compartment {locker.locker_number} status updated to {locker.status}",
        "locker_id": locker.id,
        "locker_number": locker.locker_number,
        "status": locker.status
    }


@app.post("/lockers/{locker_id_or_number}/toggle-maintenance", summary="Admin toggles maintenance status in database")
def toggle_maintenance(locker_id_or_number: str, db: Session = Depends(get_db)):
    locker = None
    if locker_id_or_number.isdigit():
        locker = db.query(models.Locker).filter(models.Locker.id == int(locker_id_or_number)).first()
    if not locker:
        locker = db.query(models.Locker).filter(models.Locker.locker_number.ilike(locker_id_or_number.strip())).first()

    if not locker:
        raise HTTPException(status_code=404, detail="Compartment not found!")

    if locker.status == "maintenance":
        locker.status = "available"
    else:
        locker.status = "maintenance"

    db.commit()
    db.refresh(locker)

    return {
        "message": f"Compartment {locker.locker_number} status is now {locker.status}",
        "locker_id": locker.id,
        "locker_number": locker.locker_number,
        "status": locker.status
    }


# customer management 
@app.post("/auth/customers/register", summary="Customer Account Registration")
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
@app.post("/auth/couriers/register", summary="Courier Account Registration")
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
def store_order(order: schemas.OrderCreate, db: Session = Depends(get_db)):

    # 1. Identify recipient customer (by ID or Username)
    customer = None
    if order.customer_id is not None:
        customer = db.query(models.Customer).filter(models.Customer.id == order.customer_id).first()
    if not customer and order.customer_username:
        customer = db.query(models.Customer).filter(
            models.Customer.username.ilike(order.customer_username.strip())
        ).first()

    if not customer:
        raise HTTPException(status_code=404, detail="Recipient customer not found in system!")

    # 2. Identify locker
    locker = None
    if order.locker_id is not None:
        locker = db.query(models.Locker).filter(models.Locker.id == order.locker_id).first()
    else:
        locker = db.query(models.Locker).filter(models.Locker.status == "available").order_by(models.Locker.id).first()

    if not locker:
        raise HTTPException(status_code=404, detail="No available locker compartment found!")

    if locker.status != "available":
        raise HTTPException(status_code=400, detail="This compartment is currently occupied or under maintenance!")

    # 3. Check existing tracking code
    existing_order = db.query(models.Order).filter(models.Order.tracking_code.ilike(order.tracking_code.strip())).first()
    if existing_order:
        raise HTTPException(status_code=400, detail="This order code already exists in the system!")

    # 4. Resolve courier
    courier_id = order.courier_id
    if courier_id is None:
        courier = db.query(models.Courier).first()
        courier_id = courier.id if courier else 1

    # 5. Create order with status 'waiting'
    new_order = models.Order(
        tracking_code=order.tracking_code.strip(),
        courier_id=courier_id,
        customer_id=customer.id,
        locker_id=locker.id,
        status="waiting"
    )
    db.add(new_order)
    locker.status = "occupied"
    db.commit()
    db.refresh(new_order)

    return {
        "message": "Package deposited successfully! The locker is now locked.",
        "order_id": new_order.id,
        "locker_id": locker.id,
        "locker_number": locker.locker_number,
        "tracking_code": new_order.tracking_code,
        "recipient": customer.username,
        "status": new_order.status
    }


# package pickup
@app.post("/orders/pickup", summary="Customer Pickup (Order Code Verification)")
def pickup_order(data: schemas.VerifyFaceAndPickup, db: Session = Depends(get_db)):
    order = db.query(models.Order).filter(
        models.Order.tracking_code.ilike(data.tracking_code.strip()),
        models.Order.status == "waiting"
    ).first()

    if not order:
        raise HTTPException(status_code=404, detail="This order is not in the locker!")

    # Identify customer requesting pickup
    customer = None
    if data.customer_id is not None:
        customer = db.query(models.Customer).filter(models.Customer.id == data.customer_id).first()
    if not customer and data.customer_username:
        customer = db.query(models.Customer).filter(
            models.Customer.username.ilike(data.customer_username.strip())
        ).first()

    if not customer:
        raise HTTPException(status_code=404, detail="Customer information not found!")

    if order.customer_id != customer.id:
        raise HTTPException(status_code=403, detail="You can not get this package!")

    if customer.face_id and data.face_id:
        if customer.face_id != data.face_id:
            raise HTTPException(status_code=401, detail="Face verification failed! The face does not match the registered recipient of this order.")

    # Transition order status to 'received'
    order.status = "received"

    # Unlock locker and set status to available
    locker = db.query(models.Locker).filter(models.Locker.id == order.locker_id).first()
    if locker:
        locker.status = "available"

    db.commit()

    return {
        "message": "Order picked up successfully! Unlocking locker number:",
        "locker_id": locker.id if locker else None,
        "locker_number": locker.locker_number if locker else None,
        "customer_name": customer.username,
        "status": order.status
    }


# admin delete/remove order from compartment
@app.delete("/orders/{tracking_code}", summary="Admin removes order from locker compartment")
def delete_order(tracking_code: str, db: Session = Depends(get_db)):
    order = db.query(models.Order).filter(models.Order.tracking_code.ilike(tracking_code.strip())).first()
    if not order:
        raise HTTPException(status_code=404, detail="Order not found!")

    order.status = "deleted"

    locker = db.query(models.Locker).filter(models.Locker.id == order.locker_id).first()
    if locker and locker.status == "occupied":
        locker.status = "available"

    db.commit()
    return {
        "message": f"Order {order.tracking_code} deleted successfully.",
        "order_id": order.id,
        "tracking_code": order.tracking_code,
        "status": order.status,
        "locker_number": locker.locker_number if locker else None
    }


@app.delete("/lockers/{locker_id_or_number}/order", summary="Admin clears order from locker compartment")
def remove_order_from_locker(locker_id_or_number: str, db: Session = Depends(get_db)):
    locker = None
    if locker_id_or_number.isdigit():
        locker = db.query(models.Locker).filter(models.Locker.id == int(locker_id_or_number)).first()
    if not locker:
        locker = db.query(models.Locker).filter(models.Locker.locker_number.ilike(locker_id_or_number)).first()

    if not locker:
        raise HTTPException(status_code=404, detail="Locker not found!")

    order = db.query(models.Order).filter(
        models.Order.locker_id == locker.id,
        models.Order.status == "waiting"
    ).first()

    if order:
        order.status = "deleted"

    locker.status = "available"
    db.commit()

    return {
        "message": f"Compartment {locker.locker_number} cleared successfully.",
        "locker_number": locker.locker_number,
        "order_tracking_code": order.tracking_code if order else None,
        "status": "deleted" if order else None
    }


@app.post("/auth/login", summary="Login")
def login(credentials: schemas.CustomerCreate, db: Session = Depends(get_db)):

    admin = db.query(models.Admin).filter(
        models.Admin.username == credentials.username,
        models.Admin.password == credentials.password
    ).first()

    if admin:
        return {
            "message": "Login successful!",
            "id": admin.id,
            "username": admin.username,
            "role": "admin", 
            "token": f"token_admin_{admin.id}"
        }
    
    customer = db.query(models.Customer).filter(
        models.Customer.username == credentials.username,
        models.Customer.password == credentials.password
    ).first()

    if customer:
        return {
            "message": "Login successful!",
            "id": customer.id,
            "username": customer.username,
            "role": "customer",
            "token": f"token_customer_{customer.id}" 
        }


    courier = db.query(models.Courier).filter(
        models.Courier.username == credentials.username,
        models.Courier.password == credentials.password
    ).first()

    if courier:
        return {
            "message": "Login successful!",
            "id": courier.id,
            "username": courier.username,
            "role": "courier",
            "token": f"token_courier_{courier.id}"
        }

    raise HTTPException(status_code=400, detail="Invalid username or password!")