# Administrator Guide

## Overview

As a system administrator, you have full access to all Unicorn HRMS features and settings.

## Dashboard Overview

The admin dashboard provides:
- System health status
- User activity metrics
- Performance indicators
- Recent alerts and notifications
- Quick access to critical functions

## User Management

### Adding New Users

1. Navigate to **Users > Add New User**
2. Fill in required information:
   - Full name
   - Email address
   - Department
   - Role
   - Employment type
3. Set initial password (user will be prompted to change)
4. Assign permissions
5. Click **Save**

### Managing User Roles

Available roles:
- **Administrator** - Full system access
- **HR Manager** - HR functions and reports
- **Department Manager** - Team management
- **Employee** - Self-service access

### Bulk User Import

1. Download CSV template: **Users > Import > Download Template**
2. Fill template with user data
3. Upload file: **Users > Import > Upload**
4. Review and confirm import

### Deactivating Users

1. Go to **Users > All Users**
2. Select user
3. Click **Actions > Deactivate**
4. Confirm deactivation

Note: Deactivated users retain historical data but cannot log in.

## Department Management

### Creating Departments

1. Navigate to **Organization > Departments**
2. Click **Add Department**
3. Enter:
   - Department name
   - Department code
   - Manager
   - Parent department (if applicable)
4. Click **Save**

### Department Hierarchy

Set up organizational structure:
- Define parent-child relationships
- Assign department managers
- Set department-specific policies

## Attendance Configuration

### Setting Work Hours

1. Go to **Settings > Attendance**
2. Define:
   - Standard work hours
   - Break times
   - Overtime rules
3. Set late/early policies

### Geofencing Setup

1. Navigate to **Settings > Geofencing**
2. Add location:
   - Location name
   - Address
   - GPS coordinates
   - Radius (in meters)
3. Assign to departments/employees

### Shift Management

1. Go to **Attendance > Shifts**
2. Create shift patterns:
   - Shift name
   - Start time
   - End time
   - Break duration
3. Assign to employees

## Leave Management

### Configuring Leave Types

1. Navigate to **Settings > Leave Types**
2. Click **Add Leave Type**
3. Configure:
   - Type name (Annual, Sick, etc.)
   - Annual quota
   - Carry forward policy
   - Approval workflow
   - Required documents

### Setting Leave Policies

1. Go to **Settings > Leave Policies**
2. Define:
   - Accrual rules
   - Eligibility criteria
   - Blackout dates
   - Notice period requirements

### Managing Leave Requests

1. Go to **Leave > Requests**
2. Review pending requests
3. Approve or reject with comments
4. View leave calendar

## Performance Management

### Setting Up KPIs

1. Navigate to **Performance > KPIs**
2. Click **Add KPI**
3. Define:
   - KPI name
   - Category
   - Target value
   - Weight
   - Evaluation criteria

### Evaluation Cycles

1. Go to **Performance > Cycles**
2. Create evaluation cycle:
   - Cycle name
   - Start/end dates
   - Participants
   - KPIs to evaluate
3. Launch cycle

### Performance Reports

Access reports:
- **Performance > Reports**
- Select report type
- Choose filters (department, date range)
- Generate and export

## System Settings

### General Settings

**Settings > General:**
- Company name and logo
- Time zone
- Date/time format
- Language preferences

### Email Configuration

**Settings > Email:**
- SMTP server settings
- Email templates
- Notification preferences

### Security Settings

**Settings > Security:**
- Password policies
- Session timeout
- Two-factor authentication
- IP restrictions

### API Configuration

**Settings > API:**
- Generate API keys
- Set rate limits
- Configure webhooks
- View API logs

## Integration Management

### CRM Integration

1. Go to **Settings > Integrations > CRM**
2. Select CRM provider
3. Enter credentials
4. Configure sync settings
5. Test connection

### Payroll Integration

1. Navigate to **Settings > Integrations > Payroll**
2. Configure payroll system
3. Map data fields
4. Set sync schedule

## Reports and Analytics

### Standard Reports

Available reports:
- Attendance summary
- Leave balance
- Performance overview
- Department statistics

### Custom Reports

1. Go to **Reports > Custom**
2. Click **Create Report**
3. Select:
   - Data sources
   - Fields to include
   - Filters
   - Grouping
4. Save and schedule

### Exporting Data

All reports can be exported in:
- PDF
- Excel
- CSV

## Backup and Maintenance

### Database Backup

1. Go to **System > Backup**
2. Click **Create Backup**
3. Select:
   - Full or incremental
   - Include uploads
4. Download backup file

### System Logs

View logs:
- **System > Logs**
- Filter by date/type
- Search for specific events
- Export logs

### System Health

Monitor:
- **System > Health**
- Database status
- Storage usage
- API performance
- Active users

## Security and Compliance

### Audit Logs

1. Navigate to **Security > Audit Logs**
2. View all system activities:
   - User logins
   - Data changes
   - Permission modifications
3. Export for compliance

### User Permissions

Review and modify:
- **Security > Permissions**
- Role-based access
- Custom permissions
- Resource restrictions

### Data Privacy

Manage:
- **Settings > Privacy**
- GDPR compliance
- Data retention policies
- User data requests

## Troubleshooting

### Common Issues

**Users can't log in:**
- Verify account is active
- Check password reset
- Review IP restrictions

**Attendance not recording:**
- Check geofencing settings
- Verify GPS permissions
- Review device compatibility

**Email not sending:**
- Verify SMTP settings
- Check email templates
- Review spam filters

### Support Resources

- System logs: **System > Logs**
- Help documentation: Built-in help system
- Technical support: support@naxus.com

## Best Practices

1. **Regular backups** - Schedule daily automated backups
2. **User training** - Provide comprehensive training
3. **Security audits** - Review logs regularly
4. **Update policies** - Keep policies current
5. **Monitor usage** - Track system performance
6. **Document changes** - Maintain change log
7. **Test updates** - Test in staging first
8. **Review permissions** - Audit access quarterly

## Keyboard Shortcuts

- `Ctrl + /` - Search
- `Ctrl + S` - Save
- `Ctrl + N` - New record
- `Alt + D` - Dashboard
- `Alt + U` - Users
- `Alt + R` - Reports

---

For HR Manager guide, see [hr-manager.md](hr-manager.md)
