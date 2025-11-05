This C program reads a text file (for example pg84.txt) and generates random sentences that resemble the style of the original text using a Markov chain approach.
The program first loads the file into memory and cleans it by replacing non-printable characters with spaces. 
It then tokenizes the text into individual words and records which words can follow each other.
These relationships are stored in arrays (tokens and succs) that act as a simple word-to-successor map.
When generating sentences, the program randomly selects a word that starts with a capital letter and keeps adding random successor words until it reaches a word that ends with a sentence terminator (? or !).
It prints two generated sentences — one ending with a question mark and one ending with an exclamation mark.
Overall, the code demonstrates:
String processing and tokenization in C
Use of arrays and dynamic memory to store relationships
Randomized text generation based on statistical word order
In short, it’s a basic text generator that learns word transitions from an input file and creates new, grammatically structured sentences inspired by that text.
