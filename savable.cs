    public class SaveAble<T>
    {
        private string _error = "";
        string LastError()
        {
            return _error;
        }
        public IList<T>? Load(string filePath)
        {
            if (!File.Exists(filePath))
            {
                Log.Error("Missing file {filePath}");
                _error = $"Missing file {filePath}";
                return null;
            }

            var content = File.ReadAllText(filePath);
            if (string.IsNullOrEmpty(content))
            {
                Log.Error("{filePath} file is empty");
                _error = $"{filePath} file is empty";
                return null;
            }

            return JsonSerializer.Deserialize<IList<T>>(content, new JsonSerializerOptions
            {
                PropertyNameCaseInsensitive = true
            })!;
        }

        public bool Save(string filename, IList<T> collection)
        {
            var options = new JsonSerializerOptions
            {
                WriteIndented = true,
                PropertyNameCaseInsensitive = true
            };
            try
            {
                var bytes = JsonSerializer.SerializeToUtf8Bytes(collection, options);
                File.WriteAllBytes(filename, bytes);
                return true;
            }
            catch (Exception ex)
            {
                _error = $"Exception:{ex.Message}";
                return false;
            }
        }

    }