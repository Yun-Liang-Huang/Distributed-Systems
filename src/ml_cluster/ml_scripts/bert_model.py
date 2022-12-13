from transformers import pipeline, BertTokenizer, BertModel
import sys

def inference(texts):
    unmasker = pipeline('fill-mask', model='bert-base-uncased')
    outputs = unmasker(texts)
    # output: 5 highest score elements
    # print("output: ", output)
    for output in outputs:
        print(output[0]['sequence'])

# get the features of a given text in PyTorch
def get_features(text):
    tokenizer = BertTokenizer.from_pretrained('bert-base-uncased')
    model = BertModel.from_pretrained("bert-base-uncased")
    # text = "Hello I'm a [MASK] model."
    encoded_input = tokenizer(text, return_tensors='pt')
    output = model(**encoded_input)
    print("output: ", output)


if (len(sys.argv) != 2):
    print('Invalid number of arguments:', len(sys.argv), 'arguments.')
    exit(1)

input_filename = sys.argv[1]

with open(input_filename) as input:
    texts = [line.rstrip() for line in input]

inference(texts)
