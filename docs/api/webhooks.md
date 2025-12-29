# Webhooks Documentation

## Overview

Unicorn HRMS supports webhooks to notify external systems of events in real-time.

## Configuration

Webhooks are configured per client installation through the admin panel or API.

### Webhook Settings

**POST** `/api/webhooks/configure`

Request:
```json
{
  "url": "https://your-system.com/webhook",
  "events": [
    "employee.created",
    "attendance.clocked-in",
    "leave.requested"
  ],
  "secret": "your-webhook-secret",
  "active": true
}
```

## Supported Events

### Employee Events

- `employee.created` - New employee added
- `employee.updated` - Employee information changed
- `employee.deleted` - Employee removed
- `employee.status_changed` - Status updated (active/inactive)

### Attendance Events

- `attendance.clocked-in` - Employee clocked in
- `attendance.clocked-out` - Employee clocked out
- `attendance.late` - Employee arrived late
- `attendance.absent` - Employee marked absent

### Leave Events

- `leave.requested` - New leave request submitted
- `leave.approved` - Leave request approved
- `leave.rejected` - Leave request rejected
- `leave.cancelled` - Leave request cancelled

### Performance Events

- `performance.evaluation_created` - New evaluation created
- `performance.evaluation_completed` - Evaluation completed
- `performance.goal_achieved` - Performance goal achieved

### System Events

- `system.backup_completed` - System backup finished
- `system.maintenance_scheduled` - Maintenance scheduled
- `system.error` - Critical system error

## Webhook Payload Format

All webhooks follow this structure:
```json
{
  "event": "employee.created",
  "timestamp": "2025-01-15T10:30:00Z",
  "webhookId": "webhook-001",
  "data": {
    // Event-specific data
  },
  "metadata": {
    "version": "1.0",
    "source": "unicorn-hrms"
  }
}
```

## Example Payloads

### Employee Created
```json
{
  "event": "employee.created",
  "timestamp": "2025-01-15T10:30:00Z",
  "webhookId": "webhook-001",
  "data": {
    "employeeId": "emp-001",
    "firstName": "Ahmed",
    "lastName": "Hassan",
    "email": "ahmed.hassan@company.com",
    "department": "Engineering",
    "position": "Software Engineer",
    "hireDate": "2025-01-15"
  }
}
```

### Attendance Clocked In
```json
{
  "event": "attendance.clocked-in",
  "timestamp": "2025-01-15T08:30:00Z",
  "webhookId": "webhook-002",
  "data": {
    "employeeId": "emp-001",
    "recordId": "att-001",
    "clockInTime": "2025-01-15T08:30:00Z",
    "location": {
      "latitude": 30.0444,
      "longitude": 31.2357
    },
    "status": "on-time"
  }
}
```

### Leave Requested
```json
{
  "event": "leave.requested",
  "timestamp": "2025-01-15T10:00:00Z",
  "webhookId": "webhook-003",
  "data": {
    "requestId": "leave-req-001",
    "employeeId": "emp-001",
    "leaveType": "Annual Leave",
    "startDate": "2025-02-01",
    "endDate": "2025-02-05",
    "totalDays": 5,
    "status": "pending"
  }
}
```

## Security

### Webhook Signature

Each webhook includes a signature header for verification:
X-Webhook-Signature: sha256=hash_value

### Verify Signature (Node.js Example)
```javascript
const crypto = require('crypto');

function verifyWebhook(payload, signature, secret) {
  const hmac = crypto.createHmac('sha256', secret);
  const digest = 'sha256=' + hmac.update(payload).digest('hex');
  return crypto.timingSafeEqual(
    Buffer.from(signature),
    Buffer.from(digest)
  );
}
```

### Verify Signature (C# Example)
```csharp
using System.Security.Cryptography;
using System.Text;

public bool VerifyWebhook(string payload, string signature, string secret)
{
    var encoding = new UTF8Encoding();
    var keyByte = encoding.GetBytes(secret);
    var messageBytes = encoding.GetBytes(payload);
    
    using (var hmac = new HMACSHA256(keyByte))
    {
        var hashMessage = hmac.ComputeHash(messageBytes);
        var hash = "sha256=" + BitConverter.ToString(hashMessage)
            .Replace("-", "").ToLower();
        return hash == signature;
    }
}
```

## Retry Logic

- Failed webhooks are retried up to 5 times
- Retry intervals: 1min, 5min, 15min, 30min, 1hour
- Webhooks failing all retries are marked as failed
- Failed webhooks can be viewed in admin panel

## Response Requirements

Your webhook endpoint should:
- Respond with HTTP 200-299 for success
- Respond within 10 seconds
- Return any status code outside 200-299 for failure

## Testing Webhooks

### Test Endpoint

**POST** `/api/webhooks/test`

Request:
```json
{
  "webhookId": "webhook-001",
  "event": "employee.created"
}
```

This sends a test payload to your configured endpoint.

## Monitoring

### Get Webhook Logs

**GET** `/api/webhooks/logs`

Query Parameters:
- `webhookId` (string)
- `status` (string): success, failed, pending
- `startDate` (date)
- `endDate` (date)

Response:
```json
{
  "data": [
    {
      "logId": "log-001",
      "webhookId": "webhook-001",
      "event": "employee.created",
      "timestamp": "2025-01-15T10:30:00Z",
      "status": "success",
      "responseCode": 200,
      "responseTime": 150,
      "retryCount": 0
    }
  ]
}
```

---

For API authentication, see [authentication.md](authentication.md)
