# NetBSD 10.1 `ls(1)` Implementation (Midterm Project)

**Môn học:** Lập trình hệ thống (System Programming)  
**Mục tiêu:** Tự triển khai công cụ `ls(1)` bằng ngôn ngữ C trên hệ điều hành NetBSD 10.1 tuân thủ đúng yêu cầu từ NetBSD General Commands Manual.

## 📁 Cấu trúc dự án
```text
midterm_ls/
├── Makefile             # Cấu hình biên dịch (GCC/Clang, -Wall -Wextra -std=c99)
├── .gitignore           # Loại trừ các file build, binary và IDE temp
├── README.md            # Tài liệu dự án
├── include/
│   ├── ls.h             # Định nghĩa cấu trúc Options, FileEntry, EntryList và kiểu dùng chung
│   ├── parse_args.h     # Giao diện module phân tích tham số dòng lệnh
│   ├── traverse.h       # Giao diện module duyệt thư mục & file (Phase 3)
│   ├── sort.h           # Giao diện module sắp xếp danh sách (Phase 4)
│   ├── display.h        # Giao diện module định dạng & in đầu ra (Phase 5)
│   └── utils.h          # Giao diện module các hàm hỗ trợ tiện ích
├── src/
│   ├── main.c           # Điểm vào chính của chương trình
│   ├── parse_args.c     # Phân tích cờ option (-AacdFfhiklnqRrSstuw) & override rules
│   ├── traverse.c       # Duyệt file/thư mục (STUB - Phase 3)
│   ├── sort.c           # Sắp xếp danh sách file (STUB - Phase 4)
│   ├── display.c        # In đầu ra (STUB - Phase 5)
│   └── utils.c        # Tiện ích bổ trợ (STUB)
└── tests/               # Thư mục chứa kịch bản kiểm thử
```

## 🚀 Trạng thái dự án (Hoàn thành Giai đoạn 6)
- [x] **Giai đoạn 1:** Khảo sát manual NetBSD 10.1 `ls.pdf` & lập kế hoạch triển khai.
- [x] **Giai đoạn 2:** Tạo bộ khung dự án, thiết lập `Makefile`, `.gitignore`, `include/ls.h`, triển khai bộ phân tích tham số `parse_arguments()` đầy đủ 19 cờ và quy tắc ghi đè (override rules).
- [x] **Giai đoạn 3:** Duyệt thư mục & thu thập dữ liệu `lstat`/`stat`, `opendir`/`readdir`, lọc cờ `-a`/`-A`/`-d`/`-f`.
- [x] **Giai đoạn 4:** Triển khai module sắp xếp danh sách (từ điển, kích thước `-S`, thời gian `-t` kèm `-c`/`-u`, đảo ngược `-r`, không sắp xếp `-f`).
- [x] **Giai đoạn 5:** Triển khai module hiển thị (Short format, Long format `-l`/`-n`, Inode `-i`, Block count `-s`, Human-readable `-h`, Kilobytes `-k`, Kiểu file `-F`, Ký tự đặc biệt `-q`/`-w`).
- [x] **Giai đoạn 6:** Triển khai tùy chọn duyệt đệ quy `-R` kết hợp đồng bộ các cờ G1–G5.
- [x] **Giai đoạn 7:** Chuẩn bị kịch bản kiểm thử toàn diện `tests/run_tests.sh` (47 ca test) và cấu hình sẵn sàng cho NetBSD 10.1 (Oracle VirtualBox).

## 🛠 Hướng dẫn triển khai và kiểm thử trên NetBSD 10.1 (Oracle VirtualBox)

### 1. Chuyển mã nguồn từ Windows sang máy ảo NetBSD
Sử dụng `scp` (nếu cấu hình Port Forwarding SSH trên VirtualBox, ví dụ cổng 2222):
```bash
# Chạy trên Windows (PowerShell):
scp -P 2222 -r E:\midterm_ls user@localhost:~/midterm_ls
```
*Hoặc đóng gói thành file tar.gz rồi tải qua FTP/HTTP:*
```powershell
tar -czf midterm_ls.tar.gz include src tests Makefile .gitignore README.md
```

### 2. Biên dịch trên NetBSD 10.1
```sh
cd ~/midterm_ls
make clean
make
```

### 3. Chạy toàn bộ test suite trên NetBSD 10.1
```sh
sh tests/run_tests.sh
```
