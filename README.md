# Midterm Project: Cài đặt lệnh `ls(1)` trên NetBSD 10.1

**Học phần:** Lập trình hệ thống (System Programming)  
**Sinh viên thực hiện:** Nguyễn Quốc Sang  
**Mã sinh viên:** 24IT228  
**Repository:** [https://github.com/SangGC/NguyenQuocSang_24IT228_midterm](https://github.com/SangGC/NguyenQuocSang_24IT228_midterm)  
**Môi trường mục tiêu:** NetBSD 10.1 (amd64) trên Oracle VirtualBox  

---

## 📖 Giới thiệu dự án

Dự án hiện thực hóa lại tiện ích dòng lệnh chuẩn `ls(1)` của hệ điều hành UNIX/BSD bằng ngôn ngữ lập trình C (C99), tuân thủ nghiêm ngặt đặc tả trong **NetBSD General Commands Manual (`ls.pdf`)**.

Chương trình được thiết kế theo kiến trúc module hóa nhiều file `.c` và `.h`, quản lý bộ nhớ an toàn (không rò rỉ bộ nhớ), xử lý đầy đủ các trường hợp biên và tương thích hoàn toàn với trình biên dịch `gcc`/`clang` cùng hệ thống BSD `make` trên NetBSD 10.1.

---

## 📁 Cấu trúc thư mục mã nguồn

```text
midterm_ls/
├── Makefile             # Cấu hình biên dịch BSD make / GNU make (CFLAGS, CPPFLAGS, -std=c99)
├── .gitignore           # Bỏ qua file đối tượng build/, binary và file nén tạm
├── README.md            # Báo cáo và tài liệu hướng dẫn dự án
├── include/
│   ├── ls.h             # Cấu trúc dữ liệu trung tâm (Options, FileEntry, EntryList)
│   ├── parse_args.h     # Giao diện phân tích tham số dòng lệnh và cờ tùy chọn
│   ├── traverse.h       # Giao diện duyệt file, phân loại toán hạng và duyệt đệ quy (-R)
│   ├── sort.h           # Giao diện thuật toán và tiêu chí sắp xếp file
│   ├── display.h        # Giao diện định dạng dữ liệu và hiển thị đầu ra
│   └── utils.h          # Tiện ích cấp phát bộ nhớ an toàn, nối chuỗi đường dẫn
├── src/
│   ├── main.c           # Điểm vào chương trình, điều phối các module
│   ├── parse_args.c     # Phân tích 19 cờ tùy chọn & xử lý quy tắc ghi đè (precedence)
│   ├── traverse.c       # Thu thập metadata qua lstat/stat, mở/đóng thư mục, đệ quy -R
│   ├── sort.c           # Sắp xếp theo tên, kích thước, thời gian mtime/ctime/atime, đảo ngược
│   ├── display.c        # Xuất short format, long format (-l/-n), căn chỉnh cột, -F, -h, -k
│   └── utils.c          # Quản lý dynamic array cho EntryList, giải phóng tài nguyên
└── tests/
    ├── .gitkeep         # Giữ thư mục trong Git
    └── run_tests.sh     # Bộ kịch bản kiểm thử tự động 48 ca test toàn diện
```

---

## ⚙️ Danh sách 19 tùy chọn đã triển khai

Chương trình hỗ trợ đầy đủ 19 cờ tùy chọn theo đúng yêu cầu đề bài và manual NetBSD 10.1:

| Cờ | Tên tùy chọn | Ý nghĩa & Hành vi chi tiết |
| :---: | :--- | :--- |
| `-A` | Almost all | Liệt kê tất cả các mục ngoại trừ `.` và `..`. Tự động bật mặc định cho super-user (`getuid() == 0`). |
| `-a` | All | Liệt kê tất cả các mục kể cả các mục bắt đầu bằng dấu chấm (`.` và `..`). |
| `-c` | Ctime | Sử dụng thời gian thay đổi trạng thái file (`st_ctime`) khi sắp xếp (`-t`) hoặc khi in (`-l`). |
| `-d` | Directories | Liệt kê bản thân thư mục như một file thông thường, không duyệt nội dung bên trong thư mục. |
| `-F` | File indicator | Thêm ký tự chỉ định kiểu file sau tên: `/` (thư mục), `*` (thực thi), `@` (symlink), `=` (socket), `\|` (FIFO), `%` (whiteout). |
| `-f` | Force / Unsorted | Không sắp xếp (giữ nguyên thứ tự đọc từ filesystem) và tự động bật `-a`. |
| `-h` | Human readable | Định dạng kích thước file bằng đơn vị dễ đọc (B, K, M, G, T, P) theo chuẩn lũy thừa 1024. |
| `-i` | Inode number | In số hiệu inode (`st_ino`) của mỗi file ở cột đầu tiên. |
| `-k` | Kilobytes | Ép đơn vị tính số block (khi dùng với `-s`) về kilobyte (1024 bytes). |
| `-l` | Long format | Định dạng chi tiết: chế độ quyền (mode), số liên kết, owner, group, kích thước, thời gian, tên file và đích symlink (`-> target`). |
| `-n` | Numeric IDs | Định dạng chi tiết tương tự `-l`, nhưng hiển thị UID và GID dạng số thay vì tra cứu tên qua `/etc/passwd` và `/etc/group`. |
| `-q` | Replace non-printable | Thay thế các ký tự không in được trong tên file bằng dấu chấm hỏi `?`. |
| `-R` | Recursive | Duyệt và hiển thị đệ quy toàn bộ cây thư mục con theo từng cấp. |
| `-r` | Reverse sort | Đảo ngược thứ tự sắp xếp kết quả. |
| `-S` | Size sort | Sắp xếp các mục theo dung lượng file giảm dần (`st_size`). |
| `-s` | Block count | In số block thực tế cấp phát trên đĩa (`st_blocks`) trước tên file. |
| `-t` | Time sort | Sắp xếp theo mốc thời gian giảm dần (mặc định là `st_mtime`; kết hợp `-c` dùng `ctime`, kết hợp `-u` dùng `atime`). |
| `-u` | Access time | Sử dụng thời gian truy cập gần nhất (`st_atime`) khi sắp xếp (`-t`) hoặc khi in (`-l`). |
| `-w` | Raw characters | In nguyên bản các ký tự không in được trong tên file (ngược với `-q`). |

### Quy tắc ghi đè lẫn nhau (Mutual Override Rules)
- **`-a` và `-A`:** Tùy chọn chỉ định sau sẽ ghi đè tùy chọn trước.
- **`-c` và `-u`:** Tùy chọn chỉ định sau sẽ quyết định mốc thời gian (`ctime` hay `atime`).
- **`-h` và `-k`:** Tùy chọn chỉ định sau sẽ quyết định đơn vị kích thước/block.
- **`-q` và `-w`:** Tùy chọn chỉ định sau sẽ quyết định cách biểu diễn ký tự không in được.
- **`-t` và `-S`:** Tùy chọn chỉ định sau sẽ quyết định tiêu chí sắp xếp chính.
- **`-d` và `-R`:** Tùy chọn chỉ định sau sẽ quyết định việc có duyệt đệ quy hay coi thư mục là file thường.

---

## 🛠 Hướng dẫn biên dịch và sử dụng

### 1. Biên dịch chương trình
Dự án tương thích với cả BSD `make` trên NetBSD và GNU `make`:

```sh
# Xóa các file build cũ
make clean

# Biên dịch chương trình (tạo binary ls_custom)
make
```

### 2. Cú pháp thực thi
```sh
./ls_custom [options] [file ...]
```

### 3. Ví dụ sử dụng thực tế
- **Liệt kê thư mục hiện tại:**
  ```sh
  ./ls_custom
  ```
- **Liệt kê chi tiết định dạng dài kèm file ẩn:**
  ```sh
  ./ls_custom -la
  ```
- **Hiển thị dung lượng dạng đọc được cho người dùng kèm số inode:**
  ```sh
  ./ls_custom -lhi
  ```
- **Hiển thị UID/GID dạng số (không phân giải tên người dùng):**
  ```sh
  ./ls_custom -n
  ```
- **Sắp xếp file theo dung lượng từ lớn đến nhỏ kèm ký hiệu loại file:**
  ```sh
  ./ls_custom -lSF
  ```
- **Sắp xếp theo thời gian sửa đổi cũ nhất lên đầu:**
  ```sh
  ./ls_custom -ltr
  ```
- **Duyệt đệ quy toàn bộ thư mục con:**
  ```sh
  ./ls_custom -R src include
  ```
- **Chỉ xem thông tin của chính thư mục (không liệt kê bên trong):**
  ```sh
  ./ls_custom -ld src include
  ```

---

## 🧪 Kết quả kiểm thử thực tế trên NetBSD 10.1

Toàn bộ 48 ca kiểm thử tự động trong kịch bản [tests/run_tests.sh](file:///e:/midterm_ls/tests/run_tests.sh) đã được thực thi và đạt **100% PASS** trên hệ điều hành NetBSD 10.1 amd64:

```text
==========================================================
 Starting ls_custom Test Suite
 Binary under test: ./ls_custom
 Host OS: NetBSD
==========================================================
Sandbox initialized at tests/sandbox

[TEST 01] Default listing without arguments                  PASS
[TEST 02] Single regular file operand                        PASS
[TEST 03] Single directory operand                           PASS
[TEST 04] Multiple directory operands                        PASS
[TEST 05] Non-existent path returns exit code > 0            PASS
[TEST 06] Mixed valid and invalid operands                   PASS
[TEST 07] Option -a (include . and ..)                       PASS
[TEST 08] Option -a (include hidden files)                   PASS
[TEST 09] Option -A (include hidden file)                    PASS
[TEST 10] Option -A (excludes . and ..)                      PASS
[TEST 11] Option -d on directory (lists as plain file)       PASS
[TEST 12] Option -d on multiple directories                  PASS
[TEST 13] Default lexicographical sort                       PASS
[TEST 14] Option -r (reverse sort)                           PASS
[TEST 15] Option -S (sort by size)                           PASS
[TEST 16] Option -Sr (sort by size reversed)                 PASS
[TEST 17] Option -t (sort by modification time)              PASS
[TEST 18] Option -tr (sort by time reversed)                 PASS
[TEST 19] Option -tc (sort by status change time ctime)      PASS
[TEST 20] Option -tu (sort by access time atime)             PASS
[TEST 21] Option -f (unsorted output)                        PASS
[TEST 22] Override -t vs -S (-t -S size wins)                PASS
[TEST 23] Override -S vs -t (-S -t time wins)                PASS
[TEST 24] Option -l (long format output)                     PASS
[TEST 25] Option -n (long format with numeric UID/GID)       PASS
[TEST 26] Option -l on empty directory                       PASS
[TEST 27] Option -i (print inode numbers)                    PASS
[TEST 28] Option -s (print block counts)                     PASS
[TEST 29] Option -sk (block counts in KB)                    PASS
[TEST 30] Option -sh (human-readable block counts)           PASS
[TEST 31] Option -lh (human-readable file sizes)             PASS
[TEST 32] Override -skh (-h wins, human readable)            PASS
[TEST 33] Override -shk (-k wins, kilobytes)                 PASS
[TEST 34] Option -F (file classification indicators)         PASS
[TEST 35] Option -q (non-printable as ?)                     PASS
[TEST 36] Option -w (raw non-printable)                      PASS
[TEST 37] Override -qw (-w wins)                             PASS
[TEST 38] Override -wq (-q wins)                             PASS
[TEST 39] Option -R (recursive listing)                      PASS
[TEST 40] Option -R on empty directory                       PASS
[TEST 41] Option -RA (recursive with hidden files)           PASS
[TEST 42] Option -Ra (recursive including . and ..)          PASS
[TEST 43] Option -Rl (recursive with long format)            PASS
[TEST 44] Option -RF (recursive with classification)         PASS
[TEST 45] Option -Rr (recursive with reverse sort)           PASS
[TEST 46] Override -Rd (-d wins, no recursion)               PASS
[TEST 47] Override -dR (-R wins, recursive)                  PASS

--- System /bin/ls detected. Comparing outputs ---
[TEST 48] Comparison with /bin/ls (default short listing)    PASS

==========================================================
 Test Suite Summary: 48 / 48 Passed (0 Failed)
==========================================================
```

### Cách chạy lại test suite:
```sh
sh tests/run_tests.sh
```

---

## 🚀 Triển khai trên môi trường NetBSD 10.1 (Oracle VirtualBox)

### 1. Chuyển mã nguồn sang máy ảo NetBSD
Gói mã nguồn được nén bằng `tar` từ máy phát triển:
```sh
tar -czf midterm_ls.tar.gz Makefile .gitignore README.md include src tests
```

Sau đó chuyển file sang máy ảo NetBSD tại `/home/quocsang/`:
```sh
scp midterm_ls.tar.gz quocsang@<IP_NETBSD>:/home/quocsang/
```

### 2. Thao tác trên máy ảo NetBSD
Đăng nhập vào tài khoản trên NetBSD và thực hiện:
```sh
cd /home/quocsang
tar -zxvf midterm_ls.tar.gz -C /home/quocsang/midterm_ls/
cd /home/quocsang/midterm_ls
make clean && make
sh tests/run_tests.sh
```
