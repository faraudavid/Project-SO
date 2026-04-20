# AI Usage Documentation

## Tool Used
- Gemini (LLM)

## Prompts Given
- **Initial Prompt:** "I need to implement a filter command for a C project. Can you generate a function `parse_condition` that splits a string like 'severity:>:5' into three parts (field, op, value)? Also, generate a `match_condition` function that compares a report struct against these parsed conditions."
- **Follow-up Prompt:** "The generated `match_condition` function is too repetitive. How can I refactor this logic to support all operators (`==`, `!=`, `<`, `<=`, `>`, `>=`) for multiple fields without writing dozens of if-else statements?"

## What was Generated
- The AI initially provided a basic `if-else` ladder for comparison.
- It provided basic string parsing logic using `sscanf`.

## What I Changed and Why
- **Refinement:** I replaced the AI's initial `if-else` block in `match_condition`. The AI's code was functional but lacked maintainability (adding a new field would have required modifying many lines).
- **Modularity:** I implemented two helper functions: `compare_int()` and `compare_str()`. This allowed me to abstract the comparison logic. Now, `match_condition` simply directs the data to the correct helper, making the code cleaner and easier to debug.
- **Robustness:** I verified that the code correctly handled `time_t` types by casting them to long integers for numeric comparison.

## What I Learned
- **AI as an Assistant:** I learned that AI is excellent at generating "boilerplate" code (like the initial structure of the functions), but it often chooses the easiest path rather than the most scalable one.
- **Critical Thinking:** It is vital to understand the code the AI generates. I learned that just because code *works* doesn't mean it is *efficient*. Taking the time to refactor the logic taught me more about C function modularity than just accepting the AI's first output.