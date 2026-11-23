from sqlalchemy import Integer, String, Column, ForeignKey
from database import Base
from sqlalchemy.orm import relationship

class Admin(Base):
    __tablename__ = 'admins'

    id = Column(Integer, primary_key=True, index=True, autoincrement=True)
    username = Column(String, unique=True, nullable=False)
    password = Column(String, nullable=False)

class Courier(Base):
    __tablename__ = 'couriers'

    id = Column(Integer, primary_key=True, index=True, autoincrement=True)
    username = Column(String, unique=True, nullable=False)
    password = Column(String, nullable=False)

    orders = relationship('Order', back_populates="courier")

class Customer(Base):
    __tablename__ = "customers"
    id = Column(Integer, primary_key=True, index=True, autoincrement=True)
    username = Column(String, nullable=False)
    password = Column(String, nullable=False)
    face_id = Column(String, nullable=True) 

    orders = relationship("Order", back_populates="customer")

class Locker(Base):
    __tablename__ = "lockers"

    id = Column(Integer, primary_key=True, index=True, autoincrement=True)
    locker_number = Column(String, unique=True, nullable=False)
    status = Column(String, default="available") #available, occupied, maintenance

class Order(Base):
    __tablename__ = "orders"

    id = Column(Integer, primary_key=True, index=True, autoincrement=True)
    tracking_code = Column(String, unique=True, index=True, nullable=False)
    courier_id = Column(Integer, ForeignKey("couriers.id"), nullable=False)
    customer_id = Column(Integer, ForeignKey("customers.id"), nullable=False)
    locker_id = Column(Integer, ForeignKey("lockers.id"), nullable=False)
    status = Column(String, default="waiting") # waiting, received

    courier = relationship("Courier", back_populates="orders")
    customer = relationship("Customer", back_populates="orders")