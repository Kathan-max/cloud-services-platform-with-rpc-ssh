import sys
from transformers import pipeline, AutoTokenizer, AutoModelForSequenceClassification
import re

def analyze_sentiment(text, model_name='nlptown/bert-base-multilingual-uncased-sentiment'):
    tokenizer = AutoTokenizer.from_pretrained(model_name)
    model = AutoModelForSequenceClassification.from_pretrained(model_name)
    sentiment_analyzer = pipeline('sentiment-analysis', model=model, tokenizer=tokenizer)
    result = sentiment_analyzer(text)
    return result[0]['label'], result[0]['score']

def generate_summary(text):
    summarizer = pipeline('summarization')
    summary = summarizer(text, max_length=100, min_length=20, length_penalty=2.0, num_beams=4, early_stopping=True)
    return summary[0]['summary_text']

def main():
    if len(sys.argv) != 2:
        print("Usage: python script.py distribution_log_filename")
        sys.exit(1)

    filename = sys.argv[1]
    line = ""
    username = filename.replace("_distribution_log.txt","")

    # Read distribution.txt to get the filename
    with open(filename, "r") as file:
        # Read the second line
        lines = file.readlines()
        if len(lines) > 1:
            line = lines[1].strip()

    # Extract file path up to CSV file name
    csv_file_name = line.find("csv")
    if csv_file_name != -1:
        line = line[:csv_file_name + 3]  # Trim the string after 'csv'

    # Open the output file for writing
    with open(f"{username}_data_an2.txt", "w") as output_file:
        # Redirect standard output to the file
        sys.stdout = output_file

        # Read and analyze the specified file
        with open(line, "r") as file:
            # Read and analyze each line in the specified file
            for line in file:
                # Split the line using both tab and comma as delimiters
                fields = re.split(r'\t|,', line.strip())
                if len(fields) == 4:
                    title = fields[2]
                    description = fields[3]

                    sentiment_label, sentiment_score = analyze_sentiment(title)

                    print(f"Video Title: {title}")
                    print(f"Sentiment: {sentiment_label} (Score: {sentiment_score})")

                    # Generate and print the summary of the video description
                    summary = generate_summary(description)
                    print(f"Summary: {summary}\n")

        # Reset standard output to the console
        sys.stdout = sys.__stdout__

if __name__ == "__main__":
    main()

