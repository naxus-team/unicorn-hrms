using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;
using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Data.Configurations
{
    public class AttendanceConfiguration : IEntityTypeConfiguration<Attendance>
    {
        public void Configure(EntityTypeBuilder<Attendance> builder)
        {
            builder.ToTable("Attendances");

            builder.HasKey(a => a.Id);

            builder.Property(a => a.EmployeeId)
                .IsRequired();

            builder.Property(a => a.Date)
                .IsRequired()
                .HasColumnType("date");

            builder.Property(a => a.CheckIn)
                .HasColumnType("time");

            builder.Property(a => a.CheckOut)
                .HasColumnType("time");

            builder.Property(a => a.Status)
                .IsRequired()
                .HasConversion<string>()
                .HasMaxLength(20);

            builder.Property(a => a.Notes)
                .HasMaxLength(500);

            builder.Property(a => a.CreatedAt)
                .IsRequired();

            builder.Property(a => a.IsDeleted)
                .HasDefaultValue(false);

            // Create composite index for employee and date
            builder.HasIndex(a => new { a.EmployeeId, a.Date })
                .IsUnique();

            // Query filter for soft delete
            builder.HasQueryFilter(a => !a.IsDeleted);
        }
    }
}