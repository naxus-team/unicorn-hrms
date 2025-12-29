# Configuration Guide

## Configuration Files

### appsettings.json

Main application configuration file located at `/opt/unicorn-hrms/appsettings.json`
```json
{
  "ConnectionStrings": {
    "DefaultConnection": "Host=localhost;Database=unicorn_hrms;Username=unicorn_admin;Password=secure_password",
    "RedisConnection": "localhost:6379"
  },
  
  "Jwt": {
    "SecretKey": "your-256-bit-secret-key-minimum-32-characters",
    "Issuer": "unicorn-hrms",
    "Audience": "unicorn-hrms-client",
    "ExpirationMinutes": 60,
    "RefreshTokenExpirationDays": 30
  },
  
  "Application": {
    "Name": "Unicorn HRMS",
    "Environment": "Production",
    "BaseUrl": "https://your-domain.com",
    "ApiVersion": "v1",
    "MaxUploadSize": 52428800,
    "AllowedFileTypes": [".pdf", ".doc", ".docx", ".jpg", ".png"]
  },
  
  "Email": {
    "SmtpServer": "smtp.gmail.com",
    "SmtpPort": 587,
    "Username": "noreply@your-company.com",
    "Password": "your-email-password",
    "FromAddress": "noreply@your-company.com",
    "FromName": "Unicorn HRMS",
    "EnableSsl": true
  },
  
  "Sms": {
    "Provider": "Twilio",
    "AccountSid": "your-account-sid",
    "AuthToken": "your-auth-token",
    "FromNumber": "+1234567890"
  },
  
  "Storage": {
    "Type": "Local",
    "LocalPath": "/opt/unicorn-hrms/uploads",
    "MaxFileSize": 52428800,
    "S3Bucket": "",
    "S3Region": "us-east-1"
  },
  
  "Logging": {
    "LogLevel": {
      "Default": "Information",
      "Microsoft": "Warning",
      "System": "Warning"
    },
    "File": {
      "Path": "/var/log/unicorn-hrms/app.log",
      "RollingInterval": "Day",
      "RetainedFileCountLimit": 30
    }
  },
  
  "Security": {
    "RequireHttps": true,
    "EnableCors": true,
    "AllowedOrigins": ["https://your-domain.com"],
    "PasswordPolicy": {
      "RequiredLength": 8,
      "RequireDigit": true,
      "RequireLowercase": true,
      "RequireUppercase": true,
      "RequireNonAlphanumeric": true
    },
    "LockoutPolicy": {
      "MaxFailedAttempts": 5,
      "LockoutDurationMinutes": 15
    }
  },
  
  "Features": {
    "EnableAttendanceGeofencing": true,
    "GeofenceRadiusMeters": 100,
    "EnableFacialRecognition": false,
    "EnablePerformanceTracking": true,
    "EnableCrmIntegration": true,
    "EnableMobileApp": true
  },
  
  "Integration": {
    "CRM": {
      "Enabled": true,
      "Provider": "Salesforce",
      "ApiUrl": "https://your-instance.salesforce.com",
      "ApiKey": "your-api-key",
      "SyncInterval": 300
    },
    "Payroll": {
      "Enabled": true,
      "Provider": "Custom",
      "ApiUrl": "https://payroll-system.com/api",
      "ApiKey": "your-api-key"
    }
  },
  
  "Notifications": {
    "Email": {
      "Enabled": true,
      "Templates": {
        "Welcome": "templates/welcome.html",
        "LeaveApproval": "templates/leave-approval.html"
      }
    },
    "Sms": {
      "Enabled": true
    },
    "Push": {
      "Enabled": true,
      "FirebaseServerKey": "your-firebase-key"
    }
  }
}
```

### Environment-Specific Configuration

**Development:** `appsettings.Development.json`
```json
{
  "Logging": {
    "LogLevel": {
      "Default": "Debug"
    }
  },
  "Security": {
    "RequireHttps": false
  }
}
```

**Production:** `appsettings.Production.json`
```json
{
  "Logging": {
    "LogLevel": {
      "Default": "Warning"
    }
  }
}
```

## Environment Variables

Set in `/opt/unicorn-hrms/.env`:
```bash
# Application
ASPNETCORE_ENVIRONMENT=Production
ASPNETCORE_URLS=http://localhost:5000

# Database
DATABASE_HOST=localhost
DATABASE_PORT=5432
DATABASE_NAME=unicorn_hrms
DATABASE_USER=unicorn_admin
DATABASE_PASSWORD=secure_password

# Security
JWT_SECRET_KEY=your-256-bit-secret-key
ENCRYPTION_KEY=your-encryption-key

# Email
SMTP_SERVER=smtp.gmail.com
SMTP_PORT=587
SMTP_USERNAME=noreply@company.com
SMTP_PASSWORD=email_password

# Storage
UPLOAD_PATH=/opt/unicorn-hrms/uploads
MAX_FILE_SIZE=52428800

# Redis
REDIS_HOST=localhost
REDIS_PORT=6379
REDIS_PASSWORD=

# Monitoring
ENABLE_METRICS=true
METRICS_PORT=9090
```

## Database Configuration

### Connection String Formats

**PostgreSQL:**
Host=localhost;Port=5432;Database=unicorn_hrms;Username=unicorn_admin;Password=password;SSL Mode=Require;Trust Server Certificate=true

**MySQL:**
Server=localhost;Port=3306;Database=unicorn_hrms;Uid=unicorn_admin;Pwd=password;SslMode=Required

### Connection Pool Settings
```json
{
  "ConnectionStrings": {
    "DefaultConnection": "Host=localhost;Database=unicorn_hrms;Username=admin;Password=pwd;Minimum Pool Size=5;Maximum Pool Size=100;Connection Lifetime=300;Connection Idle Lifetime=60"
  }
}
```

## Security Configuration

### SSL/TLS Setup
```json
{
  "Kestrel": {
    "Endpoints": {
      "Https": {
        "Url": "https://localhost:5001",
        "Certificate": {
          "Path": "/etc/ssl/certs/unicorn-hrms.pfx",
          "Password": "certificate-password"
        }
      }
    }
  }
}
```

### CORS Configuration
```json
{
  "Cors": {
    "AllowedOrigins": [
      "https://your-domain.com",
      "https://app.your-domain.com"
    ],
    "AllowedMethods": ["GET", "POST", "PUT", "DELETE", "OPTIONS"],
    "AllowedHeaders": ["*"],
    "AllowCredentials": true,
    "MaxAge": 3600
  }
}
```

## Feature Flags

Enable/disable features without code changes:
```json
{
  "FeatureFlags": {
    "AttendanceGeofencing": true,
    "FacialRecognition": false,
    "PerformanceTracking": true,
    "CrmIntegration": true,
    "MobileNotifications": true,
    "AdvancedAnalytics": true,
    "MultiLanguage": true,
    "DarkMode": true
  }
}
```

## Performance Tuning

### Cache Configuration
```json
{
  "Caching": {
    "Type": "Redis",
    "Redis": {
      "Connection": "localhost:6379",
      "InstanceName": "UnicornHRMS:",
      "DefaultExpiration": 3600
    },
    "Memory": {
      "SizeLimit": 1024,
      "CompactionPercentage": 0.2
    }
  }
}
```

### Rate Limiting
```json
{
  "RateLimiting": {
    "EnableGlobalLimiting": true,
    "Rules": [
      {
        "Endpoint": "/api/*",
        "Limit": 100,
        "Period": "1m"
      },
      {
        "Endpoint": "/api/auth/*",
        "Limit": 10,
        "Period": "1m"
      },
      {
        "Endpoint": "/api/reports/*",
        "Limit": 5,
        "Period": "1m"
      }
    ]
  }
}
```

## Logging Configuration

### File Logging
```json
{
  "Serilog": {
    "MinimumLevel": {
      "Default": "Information",
      "Override": {
        "Microsoft": "Warning",
        "System": "Warning"
      }
    },
    "WriteTo": [
      {
        "Name": "File",
        "Args": {
          "path": "/var/log/unicorn-hrms/app-.log",
          "rollingInterval": "Day",
          "retainedFileCountLimit": 30,
          "outputTemplate": "{Timestamp:yyyy-MM-dd HH:mm:ss.fff zzz} [{Level:u3}] {Message:lj}{NewLine}{Exception}"
        }
      },
      {
        "Name": "Console",
        "Args": {
          "theme": "Serilog.Sinks.SystemConsole.Themes.AnsiConsoleTheme::Code, Serilog.Sinks.Console"
        }
      }
    ]
  }
}
```

## Backup Configuration
```json
{
  "Backup": {
    "Enabled": true,
    "Schedule": "0 2 * * *",
    "RetentionDays": 30,
    "Destination": "/var/backups/unicorn-hrms",
    "IncludeDatabase": true,
    "IncludeUploads": true,
    "Compression": true,
    "Encryption": {
      "Enabled": true,
      "Key": "encryption-key"
    }
  }
}
```

## Monitoring Configuration
```json
{
  "Monitoring": {
    "HealthChecks": {
      "Enabled": true,
      "Endpoint": "/health",
      "DetailedErrors": false
    },
    "Metrics": {
      "Enabled": true,
      "Endpoint": "/metrics",
      "Port": 9090
    },
    "ApplicationInsights": {
      "Enabled": false,
      "InstrumentationKey": "your-key"
    }
  }
}
```

## Localization
```json
{
  "Localization": {
    "DefaultCulture": "en-US",
    "SupportedCultures": ["en-US", "ar-EG"],
    "ResourcesPath": "Resources"
  }
}
```

## Mobile App Configuration

### iOS Configuration

`ios/UnicornHRMS/Config.plist`:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN">
<plist version="1.0">
<dict>
    <key>ApiBaseUrl</key>
    <string>https://your-domain.com/api</string>
    <key>EnablePushNotifications</key>
    <true/>
    <key>EnableBiometrics</key>
    <true/>
</dict>
</plist>
```

### Android Configuration

`android/app/src/main/res/values/config.xml`:
```xml
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <string name="api_base_url">https://your-domain.com/api</string>
    <bool name="enable_push_notifications">true</bool>
    <bool name="enable_biometrics">true</bool>
</resources>
```

## Configuration Best Practices

1. **Never commit secrets** to version control
2. **Use environment variables** for sensitive data
3. **Different configs** for each environment
4. **Enable HTTPS** in production
5. **Regular backups** of configuration
6. **Document changes** in configuration
7. **Test configurations** before deploying
8. **Use strong passwords** for all services
9. **Enable monitoring** and logging
10. **Keep configurations** updated

## Configuration Validation

Test configuration:
```bash
# Validate JSON syntax
cat appsettings.json | jq .

# Test database connection
dotnet run --test-database

# Verify email settings
dotnet run --test-email

# Check all configurations
dotnet run --verify-config
```

---

For installation guide, see [installation.md](installation.md)
