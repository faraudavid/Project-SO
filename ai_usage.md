I used Gemini

Prompt :
I need to implement a filter command for a C project.Generate a function `parse_condition` that splits a string like 'severity:>:5' into three parts (field, op, value).After, generate a `match_condition` function that compares a report struct against these parsed conditions."


First Generation
The Ai first generated me a lot of if and else.

What i did
I replaced the AI's initial if-else block in match_condition. 
i implemented two more functions: compare_int() and compare_str(). This is a cleaner way and if i want to add a new field to be compared i don't need to 
write 4 different if-else

What I Learned
AI is very good if you use it for getting and idea or to help you when you are stucked or you have a bug that you can't find. It is very important to know exactly what AI does and to understand what it
writes because you need a solution perfect for you,not perfect for a vast majority. 
You need to understand the topic you are talking about with ai because this tool needs guidance. If you let it handle all your work without your inplication you are doomed because Ai makes a lot of
mistakes. 
