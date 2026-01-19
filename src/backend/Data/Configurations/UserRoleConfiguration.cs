using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;
using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Data.Configurations
{
    public class UserRoleConfiguration : IEntityTypeConfiguration<UserRole>
    {
        public void Configure(EntityTypeBuilder<UserRole> builder)
        {
            builder.ToTable("UserRoles");

            builder.HasKey(ur => ur.Id);

            builder.HasIndex(ur => new { ur.UserId, ur.RoleId })
                .IsUnique();

            builder.HasQueryFilter(ur => !ur.IsDeleted);
        }
    }
}