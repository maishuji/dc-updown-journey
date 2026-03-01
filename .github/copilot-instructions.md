# Copilot Instructions for UpDown Journey

## Core Development Principles

### SOLID Principles
Follow SOLID principles rigorously in all code:
- **Single Responsibility**: Each class should have one clear purpose
- **Open/Closed**: Classes should be open for extension, closed for modification
- **Liskov Substitution**: Derived classes must be substitutable for their base classes
- **Interface Segregation**: Prefer small, focused interfaces over large ones
- **Dependency Inversion**: Depend on abstractions, not concretions

### Design Pattern Naming Conventions
Design patterns MUST be explicit in class names:
- Strategy pattern: `*Strategy` (e.g., `RenderStrategy`, `MovementStrategy`)
- Factory pattern: `*Factory` (e.g., `EnemyFactory`, `ParticleFactory`)
- Observer pattern: `*Observer`, `*Subject` (e.g., `GameEventObserver`)
- Builder pattern: `*Builder` (e.g., `MonsterBuilder`)
- Singleton pattern: `*Manager` or `*System` (e.g., `ResourceManager`)
- Adapter pattern: `*Adapter` (e.g., `GraphicsAdapter`)
- Decorator pattern: `*Decorator` (e.g., `BuffDecorator`)
- Command pattern: `*Command` (e.g., `MoveCommand`)
- State pattern: `*State` (e.g., `IdleState`, `AttackState`)
- Template Method: `Abstract*` base class (e.g., `AbstractRenderer`)

### Code Quality Requirements

#### Unit Testing
- **Every feature** must have corresponding unit tests
- When implementing new features:
  1. Create tests in the `tests/` directory following the existing structure
  2. Use GoogleTest framework (already configured in project)
  3. Test files should mirror source structure: `src/foo/bar.cpp` → `tests/foo/test_bar.cpp`
  4. Aim for high code coverage on business logic
  5. Update existing tests when modifying features
- Run tests with: `make test` or the "Run Tests" VS Code task

#### Linting Compliance
- **All code must pass `make lint` without errors**
- After implementing any feature:
  1. Run `make lint` before committing
  2. Fix all cpplint warnings and errors
  3. Project uses cpplint with specific filters (see Makefile)
- Common issues to avoid:
  - Missing copyright headers
  - Line length > 80 characters
  - Improper whitespace/indentation
  - Missing include guards in headers
  - Wrong include order (C system, C++, project headers)

### Platform-Specific Considerations

#### Dreamcast Development
- Memory is constrained (16MB RAM)
- Use fixed-point math where possible (no native FPU)
- Minimize dynamic allocations
- Prefer stack allocation and object pools
- Be mindful of texture memory (8MB VRAM)

#### Cross-Platform Code
- Separate platform-specific code clearly
- Use `#ifdef DREAMCAST` for platform-specific implementations
- Keep core logic platform-agnostic
- Use dependency injection for platform-specific systems

### Code Style

#### Headers and Includes
```cpp
// Copyright notice at top of every file
// Include order: C system headers, C++ headers, project headers
#include <cstdint>
#include <memory>
#include "udjourney/core/types.hpp"
```

#### Class Structure
```cpp
class MyFeatureStrategy {  // Explicit pattern name
 public:
  // Public interface first
  virtual void Execute() = 0;
  
 protected:
  // Protected members
  
 private:
  // Private implementation
};
```

#### Naming Conventions
- Classes: `PascalCase`
- Functions/Methods: `PascalCase`
- Variables: `snake_case`
- Constants: `kPascalCase`
- Private members: `snake_case_` (trailing underscore)

### Documentation
- Use Doxygen-style comments for public APIs
- Explain **why**, not just **what**
- Document any non-obvious behavior
- Include usage examples for complex systems

### Git Workflow
When implementing features:
1. Write unit tests first (TDD when appropriate)
2. Implement the feature following SOLID principles
3. Run `make lint` and fix all issues
4. Run tests and ensure they pass
5. Update documentation if needed
6. Write commit messages following Conventional Commits standard

#### Commit Message Format
Follow the [Conventional Commits](https://www.conventionalcommits.org/) standard:

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, no logic change)
- `refactor`: Code refactoring (no feature change or bug fix)
- `perf`: Performance improvements
- `test`: Adding or updating tests
- `build`: Build system or dependency changes
- `ci`: CI/CD configuration changes
- `chore`: Other changes that don't modify src or test files

**Examples:**
```bash
feat(ai): add PatrolAIStrategy for enemy movement
fix(renderer): correct texture memory leak on Dreamcast
docs(particle): update particle system documentation
test(monster): add unit tests for MonsterBuilder
refactor(input): extract InputHandlerStrategy interface
```

### Performance Considerations
- Profile before optimizing
- Prefer data-oriented design for performance-critical code
- Keep hot paths cache-friendly
- Minimize virtual calls in tight loops

## Example: Adding a New Feature

When adding a new enemy AI strategy:

1. **Create the interface** (Dependency Inversion):
```cpp
// src/udjourney/ai/EnemyAIStrategy.hpp
class EnemyAIStrategy {
 public:
  virtual ~EnemyAIStrategy() = default;
  virtual void Update(float delta_time) = 0;
};
```

2. **Implement concrete strategy** (Explicit pattern naming):
```cpp
// src/udjourney/ai/PatrolAIStrategy.hpp
class PatrolAIStrategy : public EnemyAIStrategy {
  // Implementation
};
```

3. **Create unit tests**:
```cpp
// tests/ai/test_patrol_ai_strategy.cpp
TEST(PatrolAIStrategyTest, UpdatesPositionCorrectly) {
  // Test implementation
}
```

4. **Run quality checks**:
```bash
make lint  # Fix any errors
make test  # Ensure tests pass
```

## Tools and Commands

- **Build**: `make build-dc-debug` (Dreamcast) or use VS Code tasks
- **Test**: "Run Tests" task or from build directory
- **Lint**: `make lint`
- **Debug**: `make debug-dc` for Dreamcast debugging
- **Clean**: `make clean`

## Questions?
Refer to project documentation in `docs/` directory for system-specific details.
