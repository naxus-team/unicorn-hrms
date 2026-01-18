using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;
using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Data.Configurations
{
    public class LeaveRequestConfiguration : IEntityTypeConfiguration<LeaveRequest>
    {
        public void Configure(EntityTypeBuilder<LeaveRequest> builder)
        {
            builder.ToTable("LeaveRequests");

            builder.HasKey(l => l.Id);

            builder.Property(l => l.EmployeeId)
                .IsRequired();

            builder.Property(l => l.StartDate)
                .IsRequired()
                .HasColumnType("date");

            builder.Property(l => l.EndDate)
                .IsRequired()
                .HasColumnType("date");

            builder.Property(l => l.LeaveType)
                .IsRequired()
                .HasConversion<string>()
                .HasMaxLength(20);

            builder.Property(l => l.Reason)
                .IsRequired()
                .HasMaxLength(1000);

            builder.Property(l => l.Status)
                .IsRequired()
                .HasConversion<string>()
                .HasMaxLength(20)
                .HasDefaultValue(LeaveStatus.Pending);

            builder.Property(l => l.RejectionReason)
                .HasMaxLength(500);

            builder.Property(l => l.CreatedAt)
                .IsRequired();

            builder.Property(l => l.IsDeleted)
                .HasDefaultValue(false);

            // Create index on employee and status
            builder.HasIndex(l => new { l.EmployeeId, l.Status });

            // Query filter for soft delete
            builder.HasQueryFilter(l => !l.IsDeleted);
        }
    }
}