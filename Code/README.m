
%{
Software to accompany:
"Application of Shifted Delta Cepstral Features for GMM 
Language Identification"

Written by: 
Jonathan Lareau - Rochester Insititute of Technology - 2006
programming@jonlareau.com

Make sure your path includes the Ellis_RASTAMAT, NETLAB, and 
OTHERS Directories, otherwise MATLAB will not be able to find 
the right files to run.

JL_LID_Script supplies a simple script for doing the language 
Identification task, simply alter your directories in the script 
file as necessary. 

cepFilt	-		Cepstral Speech Enhancement Algorithm
GMMDemo - 		Gaussian Mixture Model Demo
gmmdist - 		GMM Distance Metric Function
JL_BACKEND -	Backend Data Processing for batch Training 
JL_GET_FEATS - 	Feature Extraction Function
JL_LID_Script - Script to Run the LID Task
JL_LID_Test - 	Run the Testing algorithm
JL_LID_Train - 	Run the Training Algorithm
KL - 			KL divergence metric for PDF's
makeFrames - 	Partition input data into frames
plotGMMContour- Plots a Contour Plot for a Gaussian Mixture Model
removeDuplicates - Remove duplicates in an array
VocalOnsetPoints - Vocal Onset Point detection
voicingDetecor - Speech / Non-speech detection algorithm, also acts
				a downward expanding Noise Detector.
				
Script_4_Making_Various_Plots - code used to make many of the plots 
				included in the write-up.