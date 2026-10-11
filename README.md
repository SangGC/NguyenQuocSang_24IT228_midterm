# Midterm Project: Implement `ls(1)`

**Học phần:** Lập trình hệ thống
**Họ và Tên:** Nguyễn Quốc Sang  
**Mã SV:** 24IT228  
**Repository:** https://github.com/SangGC/NguyenQuocSang_24IT228_midterm  
**Môi trường kiểm thử:** NetBSD 10.1 amd64

## 1. Giới thiệu

Dự án cài đặt một phiên bản đơn giản của lệnh UNIX `ls(1)` bằng C. Chương trình được chia thành nhiều file nguồn và header để tách các phần xử lý tham số, duyệt thư mục, sắp xếp và hiển thị thông tin file.

## 2. Các tùy chọn đã triển khai

| Tùy chọn | Chức năng |
|---|---|
| `-A` | Hiển thị các mục ẩn, không gồm `.` và `..` |
| `-a` | Hiển thị tất cả mục, gồm `.` và `..` |
| `-c` | Dùng thời gian thay đổi trạng thái file (`ctime`) khi phù hợp |
| `-d` | Hiển thị chính thư mục, không liệt kê nội dung |
| `-F` | Thêm ký hiệu để nhận biết loại file |
| `-f` | Không sắp xếp kết quả và bật hiển thị mục ẩn |
| `-h` | Hiển thị kích thước theo đơn vị dễ đọc |
| `-i` | Hiển thị số inode |
| `-k` | Dùng đơn vị kilobyte cho số block |
| `-l` | Hiển thị thông tin file ở dạng chi tiết |
| `-n` | Hiển thị UID và GID dạng số |
| `-q` | Thay ký tự không in được bằng `?` |
| `-R` | Liệt kê thư mục đệ quy |
| `-r` | Đảo ngược thứ tự sắp xếp |
| `-S` | Sắp xếp theo kích thước |
| `-s` | Hiển thị số block |
| `-t` | Sắp xếp theo thời gian |
| `-u` | Dùng thời gian truy cập (`atime`) khi phù hợp |
| `-w` | Giữ nguyên ký tự không in được khi hiển thị tên |

Khi các tùy chọn có quy tắc ghi đè được dùng cùng nhau, chương trình xử lý theo thứ tự tham số như đã cài đặt.

## 3. Cấu trúc dự án

```text
midterm_ls/
├── Makefile
├── README.md
├── .gitignore
├── include/
│   ├── ls.h
│   ├── parse_args.h
│   ├── traverse.h
│   ├── sort.h
│   ├── display.h
│   └── utils.h
├── src/
│   ├── main.c
│   ├── parse_args.c
│   ├── traverse.c
│   ├── sort.c
│   ├── display.c
│   └── utils.c
└── tests/
    └── run_tests.sh
```

## 4. Biên dịch và chạy

Thực hiện trong thư mục dự án trên NetBSD:

```sh
make clean
make
```

Sau khi biên dịch, chạy chương trình bằng cú pháp:

```sh
./ls_custom [options] [file ...]
```

Ví dụ:

```sh
./ls_custom
./ls_custom -la
./ls_custom -lhi
./ls_custom -n
./ls_custom -lSF
./ls_custom -ltr
./ls_custom -R src include
./ls_custom -ld src include
```

## 5. Kiểm thử

Chạy bộ kiểm thử bằng lệnh:

```sh
sh tests/run_tests.sh
```

Kết quả kiểm thử trên NetBSD 10.1: **48/48 test PASS**. Bộ test kiểm tra các trường hợp liệt kê file và thư mục, tùy chọn hiển thị, sắp xếp, định dạng chi tiết, duyệt đệ quy và so sánh kết quả mặc định với `/bin/ls`.