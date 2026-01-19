using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;
using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Data.Configurations
{
    public class RoleConfiguration : IEntityTypeConfiguration<Role>
    {
        public void Configure(EntityTypeBuilder<Role> builder)
        {
            builder.ToTable("Roles");

            builder.HasKey(r => r.Id);

            builder.Property(r => r.Name)
                .IsRequired()
                .HasMaxLength(50);

            builder.HasIndex(r => r.Name)
                .IsUnique();

            builder.Property(r => r.Description)
                .HasMaxLength(200);

            builder.HasMany(r => r.UserRoles)
                .WithOne(ur => ur.Role)
                .HasForeignKey(ur => ur.RoleId)
                .OnDelete(DeleteBehavior.Cascade);

            builder.HasQueryFilter(r => !r.IsDeleted);

            // Seed default roles
            builder.HasData(
                new Role { Id = 1, Name = RoleNames.SuperAdmin, Description = "Super Administrator", CreatedAt = new DateTime(2026, 1, 18, 0, 0, 0, DateTimeKind.Utc) },
                new Role { Id = 2, Name = RoleNames.Admin, Description = "Administrator", CreatedAt = new DateTime(2026, 1, 18, 0, 0, 0, DateTimeKind.Utc) },
                new Role { Id = 3, Name = RoleNames.Manager, Description = "Manager", CreatedAt = new DateTime(2026, 1, 18, 0, 0, 0, DateTimeKind.Utc) },
                new Role { Id = 4, Name = RoleNames.HR, Description = "HR personnel", CreatedAt = new DateTime(2026, 1, 18, 0, 0, 0, DateTimeKind.Utc) },
                new Role { Id = 5, Name = RoleNames.Employee, Description = "Employee", CreatedAt = new DateTime(2026, 1, 18, 0, 0, 0, DateTimeKind.Utc) }
            );
        }
    }
}