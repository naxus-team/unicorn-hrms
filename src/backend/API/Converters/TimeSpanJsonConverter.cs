using System.Text.Json;
using System.Text.Json.Serialization;

namespace UnicornHRMS.API.Converters
{
    /// <summary>
    /// JSON converter for TimeSpan to handle "HH:mm:ss" format
    /// </summary>
    public class TimeSpanJsonConverter : JsonConverter<TimeSpan>
    {
        public override TimeSpan Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
        {
            var value = reader.GetString();
            if (string.IsNullOrEmpty(value))
                return TimeSpan.Zero;

            if (TimeSpan.TryParse(value, out var result))
                return result;

            throw new JsonException($"Unable to convert \"{value}\" to TimeSpan.");
        }

        public override void Write(Utf8JsonWriter writer, TimeSpan value, JsonSerializerOptions options)
        {
            writer.WriteStringValue(value.ToString(@"hh\:mm\:ss"));
        }
    }

    /// <summary>
    /// JSON converter for nullable TimeSpan to handle "HH:mm:ss" format
    /// </summary>
    public class NullableTimeSpanJsonConverter : JsonConverter<TimeSpan?>
    {
        public override TimeSpan? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
        {
            if (reader.TokenType == JsonTokenType.Null)
                return null;

            var value = reader.GetString();
            if (string.IsNullOrEmpty(value))
                return null;

            if (TimeSpan.TryParse(value, out var result))
                return result;

            throw new JsonException($"Unable to convert \"{value}\" to TimeSpan.");
        }

        public override void Write(Utf8JsonWriter writer, TimeSpan? value, JsonSerializerOptions options)
        {
            if (value.HasValue)
                writer.WriteStringValue(value.Value.ToString(@"hh\:mm\:ss"));
            else
                writer.WriteNullValue();
        }
    }
}