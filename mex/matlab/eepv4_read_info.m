function [info] = eepv4_read_info(fn);

% eepv4_read_info reads a cnt or avg file
% and returns a structure containing data information.
%
% info = eepv4_read_info(filename)
%
% info then contains:
%
% info.channel_count ... number of channels
% info.sample_count  ... number of samples
% info.sample_rate   ... sample rate (Hz)
% info.trigger_count ... number of triggers

error('could not locate mex file');
