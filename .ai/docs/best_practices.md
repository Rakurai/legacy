# Best Practices & Development Standards

## Python Version
- Target Python 3.13 for all development. Update as needed for future dependencies.
- Use `from __future__ import annotations` in all modules to prepare for Python 3.14+ lazy type hinting.

## Type Hinting & Documentation
- Use type hints consistently across all functions, methods, and class attributes.
- Docstrings are required for public API-facing methods/classes and should be concise, providing context beyond the signature.
- Use inline comments sparingly to clarify non-obvious logic; prefer descriptive naming.

## Formatting & Linting
- Enforce code formatting and linting using `ruff` and `black`.
- Integrate `pre-commit` hooks to automate code quality checks (linting, formatting, tests).

## Data Classes & Serialization
- Use `pydantic` for data classes and schema enforcement.
- Use `sqlmodel` for serialized data classes and database models.

## Version Control
- Use Git with a simple `/dev` and `/main` branch system.
- Create feature branches for major development efforts.
- Implement more complex workflows and review processes as the team grows.

## Testing
- Use `pytest` for unit testing of each developed module.
- Incorporate `pytest-randomly` to detect inter-test dependencies.
- Use `coverage.py` to monitor and improve test coverage.
- Avoid excessive or brittle tests during early refactoring.

## Documentation
- Maintain all documentation in Markdown, except for minimal docstrings and inline comments.
- Generate HTML documentation using tools like `pdoc` or `Sphinx` as needed.

## Configuration Management
- Organize configuration files within a dedicated `config/` directory.
- Use `pydantic`'s `BaseSettings` for structured and validated configurations.
- Support environment-specific configurations using `.env` files and environment variables.
- For modules with a main entry point, use the following pattern:

```python
# ...imports...
def parse_args():
    # ...add args...
    return parsed_arguments

def main(args):
    # ...executable code...

if __name__ == "__main__":
    args = parse_args()
    main(args)
```
- Consider using a CLI tool (e.g., `typer`, `click`) for consistent argument parsing.

## Logging
- Use `from loguru import logger as log` in each source file for logging.
- Avoid using print statements; remove or comment out any temporary debugging prints before committing.
- Configure logging at the start of the application to ensure consistent settings across modules.

## Error Handling
- Handle errors at the lowest level where they can be corrected using `try/except/finally`.
- Only propagate exceptions that cannot be handled in the current scope.
- Use exceptions for error signaling; avoid C-style error codes.
- Returning `None` is acceptable when an operation cannot be fulfilled as a normal result (not as an error signal).

## Security & Privacy
- Do not expose user data; keep it confidential.
- Do not publish game content; keep it confidential.
- Use virtual environments to isolate project dependencies.
- Regularly update dependencies and monitor for vulnerabilities using tools like `pip-audit` or `Safety`.
- Avoid hardcoding secrets; use environment variables or secret management tools.
- Implement scanning tools to detect accidental inclusion of secrets in the codebase.

## Tooling and Automation
- Use `mypy` or `pyright` for static type checking.
- Integrate code smell detection tools like `PyExamine` as needed.

## Project Structure
- Follow consistent naming conventions: snake_case for functions/variables, PascalCase for classes, UPPER_CASE for constants.
- Organize configuration and environment files in a `config/` directory.
- Avoid global state; prefer dependency injection or explicit state passing.
- Limit function/method length and keep them focused.
- Use context managers for resource management.
- Avoid bare except; always catch specific exceptions.
- Avoid circular imports by structuring modules appropriately.

## Accessibility & Internationalization
- Be open to accessibility improvements as the project evolves.
- Use Unicode for all strings; the game will be English-only for now.

---

This document will evolve as the project and team grow. Revisit and update as needed.
