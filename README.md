# Updown Journey Project
An independent 2D game currently in development for the Sega Dreamcast, following the journey of a human character traveling from top to bottom. 

## Known issues

##### Ninja generator issue
- kos toolchain seems to have issues with ninja (`floating-point exception`)
- This issue is oddly not visible in certain circumstances ( e.g when building and calling just after a custom target)
    - Didn't find the root cause, but as a workaround, we can use the `Unix Makefiles` generator instead of `Ninja` in the CMake configuration.

In `settings.json`:
```json
{
    "cmake.generator": "Unix Makefiles"
}
```