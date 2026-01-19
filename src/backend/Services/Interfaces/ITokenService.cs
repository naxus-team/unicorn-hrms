using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Services.Interfaces
{
    public interface ITokenService
    {
        string GenerateAccessToken(User user, IEnumerable<string> roles);
        string GenerateRefreshToken();
        (bool isValid, string? username) ValidateToken(string token);
    }
}