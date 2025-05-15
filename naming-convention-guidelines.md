# Parameter Naming Convention Guidelines

## Purpose  
To clearly communicate the intended use of function and method parameters by using consistent naming conventions and C++ type features, improving code readability and maintainability.

---

## 1. Raw Pointers (`T*`)

Raw pointers do **not** inherently express whether the data is input, output, or both. Use prefixes to clarify intent:

| Prefix | Meaning                          | Example                       |
|--------|---------------------------------|------------------------------|
| `i`    | Input only (read-only)           | `int* iBuffer`               |
| `o`    | Output only (write-only)         | `float* oResult`             |
| `io`   | Input and output (read-write)    | `char* ioString`             |

**Note:**  
- Always document pointer ownership and nullability separately.  
- Prefer safer alternatives (references, smart pointers) when possible.

---

## 2. References (`T&` and `const T&`)

Use C++’s type system to express intent clearly without prefixes:

| Type           | Meaning                          | Example                  |
|----------------|---------------------------------|--------------------------|
| `const T&`     | Input only (read-only)           | `const std::string& name` |
| `T&`           | Output or input/output (modifiable) | `int& outValue`            |

---

## 3. Smart Pointers (`std::unique_ptr`, `std::shared_ptr`)

- Prefer **no prefixes**, use type to communicate ownership and lifetime semantics.
- Document clearly whether the pointer is owned or shared.

---

## 4. Built-in Types (`int`, `float`, etc.)

- Use `i` prefix for input parameters only when helpful to distinguish.
- Otherwise, names should be descriptive without prefixes (e.g., `count`, `index`).

---

## 5. Optional Parameters and Nullability


For parameters that can be optional (nullable pointers), indicate nullability clearly in documentation and optionally in the name:

| Prefix/Suffix            | Meaning                          | Example                          |
|-------------------------|---------------------------------|---------------------------------|
| `ioBufferNullable`       | Nullable input-output raw pointer | `char* ioBufferNullable`         |
| `iDataNullable`          | Nullable input-only raw pointer   | `const int* iDataNullable`       |

Use modern C++ alternatives like `std::optional<T>` or smart pointers (`std::unique_ptr`, `std::shared_ptr`) where possible to better express optionality.

### Examples

```cpp
// Nullable raw pointer input-output parameter
void transformData(char* ioBufferNullable, size_t size);

// Nullable input pointer ()
void logMessage(const std::string* iMessageNullable);

// Using std::optional instead of nullable raw pointer
#include <optional>
void processValue(std::optional<int> iValueOptional);

// Raw pointer input-output
void processData(int* ioData, size_t size);

// Reference input (read-only)
void printMessage(const std::string& iMessage);

// Reference output
void updateCount(int& oCount);

// Smart pointer ownership (no prefix)
void setHandler(std::unique_ptr<Handler> handler);
