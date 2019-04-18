clear all;
inputVideo = VideoReader('defectSample.avi');

mov = VideoWriter('t1.avi');
open(mov)

while hasFrame(inputVideo)
    writeVideo(mov,readFrame(inputVideo));
end

inputVideo1 = VideoReader('defectSample.avi');
while hasFrame(inputVideo1)
    writeVideo(mov,readFrame(inputVideo1));
end
close(mov)
