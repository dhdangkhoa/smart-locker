# Hướng Dẫn Chạy Dự Án (Docker Setup)

Dự án đã được đóng gói hoàn toàn bằng Docker. Chỉ cần thực hiện các bước dưới đây để chạy cả App và Database (PostgreSQL) mà không cần cài đặt thêm môi trường cục bộ.

---

## Yêu Cầu Tiền Đề

Trước khi bắt đầu, đảm bảo máy bạn đã cài đặt:
- **[Docker Desktop]** (cho Windows/macOS) hoặc **Docker Engine** (cho Linux).
- **Git** để tải mã nguồn.


## Các Bước Khởi Chạy Dự Án

### Bước 1: Clone dự án về máy
Mở Terminal/CMD và chạy lệnh:
```bash
git clone https://github.com/dhdangkhoa/smart-locker
cd smart-locker
```

### Bước 2: Khởi chạy dự án bằng Docker
```bash
docker compose up --build -d
```