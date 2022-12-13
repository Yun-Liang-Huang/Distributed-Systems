from transformers import ViTFeatureExtractor, ViTForImageClassification
from PIL import Image
import requests
import torch
import sys

if len(sys.argv) != 2:
    print("Invalid arguement number")
    print("Example: python3 vit_model.py <input filename>")
    exit(1)

input_filename = sys.argv[1]

with open(input_filename) as input:
    urls = [line.rstrip() for line in input]
    images = [Image.open(requests.get(image_url, stream=True).raw) for image_url in urls]

feature_extractor = ViTFeatureExtractor.from_pretrained('google/vit-base-patch16-224')
model = ViTForImageClassification.from_pretrained('google/vit-base-patch16-224')

inputs = feature_extractor(images=images, return_tensors="pt")
outputs = model(**inputs)
logits = outputs.logits

# model predicts one of the 1000 ImageNet classes
for logit in logits:
    predicted_class_idx = logit.argmax(-1).item()
    print(model.config.id2label[predicted_class_idx])
