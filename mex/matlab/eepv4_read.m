function [data] = eepv4_read(fn);

% eepv4_read reads data from aa cnt or avg file
%
% data = eepv4_read_info(filename, sample1, sample2)
%
% where sample1 and sample2 are the begin and end sample of the data
% to be read, starting at zero sample2 being non-inclusive
%
% data then contains:
%
% data.samples  ... array [nchan x npnt] containing eeg data (uV) 
% data.triggers ... array [ offset, code ] trigger info, where each trigger has an offset(in samples) and a code

error('could not locate mex file');
