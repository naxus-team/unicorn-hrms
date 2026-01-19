using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;
using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Data.Configurations
{
    public class EmployeeConfiguration : IEntityTypeConfiguration<Employee>
    {
        public void Configure(EntityTypeBuilder<Employee> builder)
        {
            builder.ToTable("Employees");

            builder.HasKey(e => e.Id);

            builder.Property(e => e.EmployeeCode)
                .IsRequired()
                .HasMaxLength(20);

            builder.HasIndex(e => e.EmployeeCode)
                .IsUnique();

            builder.Property(e => e.FirstName)
                .IsRequired()
                .HasMaxLength(100);

            builder.Property(e => e.LastName)
                .IsRequired()
                .HasMaxLength(100);

            builder.Property(e => e.Email)
                .IsRequired()
                .HasMaxLength(255);

            builder.HasIndex(e => e.Email)
                .IsUnique();

            builder.Property(e => e.PhoneNumber)
                .HasMaxLength(20);

            builder.Property(e => e.Position)
                .IsRequired()
                .HasMaxLength(100);

            builder.Property(e => e.Salary)
                .HasColumnType("decimal(18,2)");

            builder.Property(e => e.Address)
                .HasMaxLength(500);

            builder.Property(e => e.City)
                .HasMaxLength(100);

            builder.Property(e => e.Country)
                .HasMaxLength(100);

            builder.Property(e => e.PostalCode)
                .HasMaxLength(20);

            builder.Property(e => e.ProfilePicture)
                .HasMaxLength(500);

            builder.Property(e => e.CreatedAt)
                .IsRequired();

            builder.Property(e => e.IsDeleted)
                .HasDefaultValue(false);

            builder.Property(e => e.IsActive)
                .HasDefaultValue(true);

            builder.HasOne(e => e.Manager)
                .WithMany(e => e.Subordinates)
                .HasForeignKey(e => e.ManagerId)
                .OnDelete(DeleteBehavior.Restrict);

            builder.HasMany(e => e.Attendances)
                .WithOne(a => a.Employee)
                .HasForeignKey(a => a.EmployeeId)
                .OnDelete(DeleteBehavior.Restrict);

            builder.HasMany(e => e.LeaveRequests)
                .WithOne(l => l.Employee)
                .HasForeignKey(l => l.EmployeeId)
                .OnDelete(DeleteBehavior.Restrict);

            builder.HasQueryFilter(e => !e.IsDeleted);
        }
    }
}