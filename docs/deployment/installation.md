# Installation Guide

## Prerequisites

Before installing Unicorn HRMS, ensure you have:

- Server meeting [system requirements](requirements.md)
- Database server (PostgreSQL or MySQL) installed
- SSL certificate for HTTPS
- Access to server terminal/command line
- Domain name configured

## Installation Steps

### 1. Database Setup

#### PostgreSQL
```bash
# Install PostgreSQL
sudo apt update
sudo apt install postgresql postgresql-contrib

# Create database and user
sudo -u postgres psql

CREATE DATABASE unicorn_hrms;
CREATE USER unicorn_admin WITH ENCRYPTED PASSWORD 'your_secure_password';
GRANT ALL PRIVILEGES ON DATABASE unicorn_hrms TO unicorn_admin;
\q
```

#### MySQL
```bash
# Install MySQL
sudo apt update
sudo apt install mysql-server

# Secure installation
sudo mysql_secure_installation

# Create database and user
sudo mysql

CREATE DATABASE unicorn_hrms CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER 'unicorn_admin'@'localhost' IDENTIFIED BY 'your_secure_password';
GRANT ALL PRIVILEGES ON unicorn_hrms.* TO 'unicorn_admin'@'localhost';
FLUSH PRIVILEGES;
EXIT;
```

### 2. Install .NET Runtime
```bash
# Add Microsoft package repository
wget https://packages.microsoft.com/config/ubuntu/22.04/packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
rm packages-microsoft-prod.deb

# Install .NET 8.0 Runtime
sudo apt update
sudo apt install -y dotnet-runtime-8.0 aspnetcore-runtime-8.0
```

### 3. Deploy Application Files
```bash
# Create application directory
sudo mkdir -p /opt/unicorn-hrms
sudo chown $USER:$USER /opt/unicorn-hrms

# Extract application files
cd /opt/unicorn-hrms
tar -xzf unicorn-hrms-v1.0.0.tar.gz

# Set permissions
sudo chown -R www-data:www-data /opt/unicorn-hrms
sudo chmod -R 755 /opt/unicorn-hrms
```

### 4. Configuration

#### Database Connection

Edit `/opt/unicorn-hrms/appsettings.json`:
```json
{
  "ConnectionStrings": {
    "DefaultConnection": "Host=localhost;Database=unicorn_hrms;Username=unicorn_admin;Password=your_secure_password"
  },
  "Jwt": {
    "SecretKey": "your-256-bit-secret-key-here",
    "Issuer": "unicorn-hrms",
    "Audience": "unicorn-hrms-client",
    "ExpirationMinutes": 60
  },
  "Application": {
    "Name": "Unicorn HRMS",
    "Environment": "Production",
    "BaseUrl": "https://your-domain.com"
  }
}
```

#### Environment Variables

Create `/opt/unicorn-hrms/.env`:
```bash
ASPNETCORE_ENVIRONMENT=Production
ASPNETCORE_URLS=http://localhost:5000
DATABASE_CONNECTION_STRING=Host=localhost;Database=unicorn_hrms;Username=unicorn_admin;Password=your_secure_password
JWT_SECRET_KEY=your-256-bit-secret-key-here
```

### 5. Database Migration
```bash
cd /opt/unicorn-hrms
dotnet ef database update
```

Or use provided migration scripts:
```bash
# PostgreSQL
psql -U unicorn_admin -d unicorn_hrms -f database/migrations/001_initial_schema.sql

# MySQL
mysql -u unicorn_admin -p unicorn_hrms < database/migrations/001_initial_schema.sql
```

### 6. Create Systemd Service

Create `/etc/systemd/system/unicorn-hrms.service`:
```ini
[Unit]
Description=Unicorn HRMS Application
After=network.target

[Service]
Type=notify
User=www-data
Group=www-data
WorkingDirectory=/opt/unicorn-hrms
ExecStart=/usr/bin/dotnet /opt/unicorn-hrms/UnicornHRMS.dll
Restart=always
RestartSec=10
KillSignal=SIGINT
SyslogIdentifier=unicorn-hrms
Environment=ASPNETCORE_ENVIRONMENT=Production
Environment=DOTNET_PRINT_TELEMETRY_MESSAGE=false

[Install]
WantedBy=multi-user.target
```

Enable and start service:
```bash
sudo systemctl daemon-reload
sudo systemctl enable unicorn-hrms
sudo systemctl start unicorn-hrms
sudo systemctl status unicorn-hrms
```

### 7. Configure Web Server

#### Nginx Configuration

Create `/etc/nginx/sites-available/unicorn-hrms`:
```nginx
server {
    listen 80;
    server_name your-domain.com;
    return 301 https://$server_name$request_uri;
}

server {
    listen 443 ssl http2;
    server_name your-domain.com;

    ssl_certificate /etc/ssl/certs/your-domain.crt;
    ssl_certificate_key /etc/ssl/private/your-domain.key;
    ssl_protocols TLSv1.2 TLSv1.3;
    ssl_ciphers HIGH:!aNULL:!MD5;

    client_max_body_size 50M;

    location / {
        proxy_pass http://localhost:5000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection keep-alive;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_cache_bypass $http_upgrade;
    }

    location /api/ {
        proxy_pass http://localhost:5000/api/;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection keep-alive;
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
        
        # API rate limiting
        limit_req zone=api_limit burst=20 nodelay;
    }
}

# Rate limiting zone
limit_req_zone $binary_remote_addr zone=api_limit:10m rate=100r/m;
```

Enable site:
```bash
sudo ln -s /etc/nginx/sites-available/unicorn-hrms /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

### 8. Configure Firewall
```bash
# Allow SSH
sudo ufw allow 22/tcp

# Allow HTTP and HTTPS
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp

# Enable firewall
sudo ufw enable
sudo ufw status
```

### 9. SSL Certificate (Let's Encrypt)
```bash
# Install Certbot
sudo apt install certbot python3-certbot-nginx

# Obtain certificate
sudo certbot --nginx -d your-domain.com

# Auto-renewal test
sudo certbot renew --dry-run
```

### 10. Initial Setup

Access the application at `https://your-domain.com`

Default admin credentials:
- Username: `admin@unicorn-hrms.com`
- Password: `Admin@123`

**IMPORTANT:** Change the default password immediately after first login.

### 11. Web Application Deployment
```bash
# Install Node.js
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt install -y nodejs

# Deploy web app
cd /opt/unicorn-hrms/web-app
npm ci --production
npm run build

# Configure Nginx to serve static files
# Add to Nginx configuration:
location /app {
    root /opt/unicorn-hrms/web-app/dist;
    try_files $uri $uri/ /index.html;
}
```

### 12. Mobile App Configuration

Update mobile app API endpoints:

iOS: `ios/UnicornHRMS/Config.swift`
Android: `android/app/src/main/res/values/config.xml`
```xml
https://your-domain.com/api
```

## Post-Installation

### 1. Verify Installation
```bash
# Check service status
sudo systemctl status unicorn-hrms

# Check logs
sudo journalctl -u unicorn-hrms -n 50 -f

# Test API
curl https://your-domain.com/api/health
```

### 2. Configure Backup
```bash
# Create backup script
sudo nano /opt/unicorn-hrms/scripts/backup.sh
```
```bash
#!/bin/bash
BACKUP_DIR="/var/backups/unicorn-hrms"
DATE=$(date +%Y%m%d_%H%M%S)

# Database backup
pg_dump -U unicorn_admin unicorn_hrms > "$BACKUP_DIR/db_$DATE.sql"

# Files backup
tar -czf "$BACKUP_DIR/files_$DATE.tar.gz" /opt/unicorn-hrms/uploads

# Keep only last 30 days
find $BACKUP_DIR -mtime +30 -delete
```

Add to crontab:
```bash
sudo crontab -e
0 2 * * * /opt/unicorn-hrms/scripts/backup.sh
```

### 3. Monitoring Setup

Install monitoring tools:
```bash
# Install monitoring agent
sudo apt install prometheus-node-exporter

# Configure monitoring
sudo systemctl enable prometheus-node-exporter
sudo systemctl start prometheus-node-exporter
```

## Troubleshooting

### Service Won't Start
```bash
# Check logs
sudo journalctl -u unicorn-hrms -xe

# Check permissions
ls -la /opt/unicorn-hrms

# Verify .NET installation
dotnet --info
```

### Database Connection Issues
```bash
# Test database connection
psql -U unicorn_admin -d unicorn_hrms -h localhost

# Check database logs
sudo tail -f /var/log/postgresql/postgresql-14-main.log
```

### Performance Issues
```bash
# Check resource usage
htop

# Monitor application
sudo systemctl status unicorn-hrms
```

## Updating

See [CHANGELOG.md](../../CHANGELOG.md) for version-specific update instructions.
```bash
# Stop service
sudo systemctl stop unicorn-hrms

# Backup current version
sudo cp -r /opt/unicorn-hrms /opt/unicorn-hrms.backup

# Extract new version
sudo tar -xzf unicorn-hrms-v1.1.0.tar.gz -C /opt/

# Run migrations
cd /opt/unicorn-hrms
dotnet ef database update

# Start service
sudo systemctl start unicorn-hrms
```

---

For configuration options, see [configuration.md](configuration.md)
