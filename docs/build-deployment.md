# Build and Deployment Guide

## Build System Overview

### CMake Configuration
The project uses CMake with FetchContent for dependency management:

```cmake
cmake_minimum_required(VERSION 3.13...3.22)
include(FetchContent)
include(ExternalProject)

project(app)
```

### Dependencies
- **nlohmann/json**: JSON parsing library
- **cpr**: HTTP client library
- **Wt**: Core web framework (system dependency)

### Build Targets
- `app`: Main executable
- `run`: Custom target for running the application

## Build Commands

### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/debug
make -j$(nproc) -C build/debug
```

### Release Build
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/release
make -j$(nproc) -C build/release
```

### Clean Build
```bash
rm -rf build/
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/debug
make -j$(nproc) -C build/debug
```

## Running the Application

### Development Mode
```bash
cd build/debug
./app --docroot ../../ -c ../../wt_config.xml --http-address 0.0.0.0 --http-port 9020
```

### Using CMake Run Target
```bash
make -C build/debug run
```

### Custom Configuration
```bash
./app --http-port=8080 --docroot=/path/to/static --config=/path/to/config.xml
```

## Configuration

### wt_config.xml
Main configuration file for Wt application settings:
- Server configuration
- Resource paths
- Session management
- Authentication settings

### Static Resources
The `--docroot` parameter should point to the directory containing:
- `static/` - CSS, JavaScript, images
- `resources/` - Wt framework resources
- `favicon.svg` - Site favicon

## Database Setup

### SQLite (Default)
The application uses SQLite by default. Database file will be created automatically:
- Location: `build/debug/dbo.db` (or build directory)
- No additional setup required

### Database Migration
The application handles schema creation automatically through Wt::Dbo:
```cpp
session.createTables();
```

## Deployment Options

### 1. Standalone HTTP Server
```bash
./app --http-port=80 --docroot=/var/www/myapp --config=/etc/myapp/wt_config.xml
```

### 2. HTTPS with SSL
```bash
./app --https-port=443 \
      --ssl-certificate=/path/to/cert.pem \
      --ssl-private-key=/path/to/key.pem \
      --docroot=/var/www/myapp
```

### 3. FastCGI Deployment
```bash
./app --fcgi-socket=/tmp/myapp.sock
```

Then configure your web server (nginx/Apache) to proxy to the FastCGI socket.

### 4. Reverse Proxy Setup
Use with nginx or Apache as reverse proxy:
```bash
./app --http-port=8080 --http-address=127.0.0.1
```

## Environment Setup

### System Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install libwt-dev libwtdbo-dev libwtdbosqlite-dev libboost-regex-dev

# CentOS/RHEL
sudo yum install wt-devel

# macOS
brew install wt
```

### Development Tools
```bash
# Install build tools
sudo apt-get install build-essential cmake git

# Optional: Install clang-format for code formatting
sudo apt-get install clang-format
```

## Docker Deployment

### Dockerfile Example
```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libwt-dev libwtdbo-dev libwtdbosqlite-dev \
    libboost-regex-dev cmake build-essential

COPY . /app
WORKDIR /app

RUN cmake -DCMAKE_BUILD_TYPE=Release -S . -B build && \
    make -j$(nproc) -C build

EXPOSE 8080
CMD ["./build/app", "--http-port=8080", "--docroot=.", "--http-address=0.0.0.0"]
```

### Docker Compose
```yaml
version: '3.8'
services:
  app:
    build: .
    ports:
      - "8080:8080"
    volumes:
      - ./data:/app/data
    environment:
      - WT_CONFIG_XML=/app/wt_config.xml
```

## Performance Optimization

### Compiler Optimizations
```bash
# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" -S . -B build/release
```

### Resource Optimization
- Enable gzip compression in web server
- Minimize CSS/JavaScript files
- Optimize image assets
- Use CDN for external resources

### Database Optimization
- Create appropriate indexes
- Use connection pooling for high traffic
- Consider PostgreSQL for production

## Monitoring and Logging

### Application Logging
```cpp
Wt::WApplication::instance()->log("info") << "Application started";
```

### Log Configuration in wt_config.xml
```xml
<configuration>
    <logging>
        <file>/var/log/myapp/app.log</file>
        <level>info</level>
    </logging>
</configuration>
```

### System Monitoring
- Monitor memory usage
- Track HTTP response times
- Monitor database connections
- Use tools like htop, iotop for system metrics

## Troubleshooting

### Common Build Issues
1. **Missing Wt libraries**: Install development packages
2. **CMake version**: Ensure CMake 3.13+
3. **Compiler errors**: Check C++17 support

### Runtime Issues
1. **Port already in use**: Change port or kill existing process
2. **Permission denied**: Check file permissions for docroot
3. **Database locked**: Ensure no other instances are running

### Debug Build
```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g -O0" -S . -B build/debug

# Run with debugger
gdb ./build/debug/app
```

### Memory Debugging
```bash
# Build with sanitizers
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address -g" -S . -B build/debug

# Run with valgrind
valgrind --leak-check=full ./build/debug/app
```

## Security Considerations

### SSL/TLS Configuration
- Use strong cipher suites
- Enable HSTS headers
- Regular certificate renewal

### Application Security
- Input validation on all forms
- SQL injection prevention (Wt::Dbo handles this)
- XSS prevention through proper escaping
- CSRF protection (built into Wt)

### File Upload Security
- Validate file types and sizes
- Scan uploaded files for malware
- Store uploads outside web root
- Implement rate limiting

## Backup and Recovery

### Database Backup
```bash
# SQLite backup
cp dbo.db dbo.db.backup

# Automated backup script
sqlite3 dbo.db ".backup backup-$(date +%Y%m%d).db"
```

### Application Backup
- Source code in version control
- Configuration files
- Static assets
- Database files
- SSL certificates
