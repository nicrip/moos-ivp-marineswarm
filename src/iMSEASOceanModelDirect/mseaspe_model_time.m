function [MODEL_START_TIME, MODEL_END_TIME, MODEL_DURATION] = mseaspe_model_time(MODEL_FILEPATH)
  ncids = readmseaspe_moossafir(MODEL_FILEPATH);
  ncid = ncids{1}{1};
  model_start_time_vec = get_petim0(ncid);                                    %get MSEAS model start date/time
  MODEL_DURATION = ncid{'time'}(end) - ncid{'time'}(1);                       %get MSEAS model duration
  readmseaspe_moossafir();
  epoch = [1970 1 1 0 0 0];
  MODEL_START_TIME = 86400*(datenum(model_start_time_vec) - datenum(epoch));  %convert MSEAS model start date/time to unix seconds
  MODEL_END_TIME = MODEL_START_TIME + MODEL_DURATION;                          %calculate MSEAS model end date/time in unix seconds
endfunction

function pe_tim0 = get_petim0(ncid);
  %% EXTRACTED FROM radmseaspe_octave.m %%
  %model start time read
  pe_tim0 = [];
  tim0str = ncid{'time'}.units(:);
  if isempty(tim0str)
     disp (' ');
     disp ('***Error:  GET_PETIM0 - unable to read time units');
     disp (' ');
     return;
  end;
  sind = findstr (upper(tim0str),'SINCE');
  if isempty(sind)
     pe_tim0 = try_pejdate0 (ncid);
     return;
  end;
  if (length(tim0str)<=(sind+5))
     pe_tim0 = try_pejdate0 (ncid);
     return;
  end;
  tim0str(1:(sind+4)) = [];
  tim0str = fliplr(deblank(fliplr(deblank(tim0str))));
  ind = findstr(tim0str,'-');
  if ~isempty(ind)
     tim0str(ind(1:2)) = ' ';
  end;
  ind = findstr(tim0str,':');
  if ~isempty(ind)
     tim0str(ind) = ' ';
  end;
  twk = str2num (tim0str');
  pe_tim0 = twk(1:6);
  gmtshft = twk(7:length(twk));
  repind = 2:length(gmtshft);
  gmtshft(repind) = abs(gmtshft(repind)).*sign(gmtshft(1));
  repind = 3 + (1:length(gmtshft));
  pe_tim0(repind) = pe_tim0(repind) + gmtshft;
endfunction
