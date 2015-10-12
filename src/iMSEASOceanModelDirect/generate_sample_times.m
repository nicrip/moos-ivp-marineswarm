function [MODEL_SAMPLE_TIMES] = generate_sample_times(MODEL_TIME_OFFSET, MODEL_TIME_REQUEST, MODEL_START_TIME, MODEL_END_TIME, MODEL_DURATION, NUM_SAMPLES)
  epoch = [1970 1 1 0 0 0];
  offset_time = MODEL_TIME_OFFSET - MODEL_START_TIME;                         %MODEL_TIME_OFFSET is always current date/time - calculate difference between time now and MSEAS model start time
  adjusted_sample_time = MODEL_TIME_REQUEST - offset_time;                    %MODEL_TIME_REQUEST is usually the elapsed MOOSTime+(specified offset in seconds)
  if(adjusted_sample_time < MODEL_START_TIME)
    adjusted_sample_time = adjusted_sample_time + MODEL_DURATION;             %if given (specified offset in seconds) is negative, the first request will wrap from end of MSEAS model
  end
  if(adjusted_sample_time > MODEL_END_TIME)
    adjusted_sample_time = adjusted_sample_time - MODEL_DURATION;             %if request has passed end of MSEAS model time, the request will wrap again from start of MSEAS model
  end
  sample_time_vec = datevec(datenum(epoch+[0 0 0 0 0 adjusted_sample_time])); %convert unix time back to date/time vector required by readmseaspe_octave
  times = [sample_time_vec];
  MODEL_SAMPLE_TIMES = repmat(times, [NUM_SAMPLES, 1]);
endfunction
