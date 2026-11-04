foreach plugin : ['rkximage', 'rockchipmpp', 'kmssrc', 'dmaheap']
  if not get_option(plugin).disabled()
    subdir(plugin)
  endif
endforeach

if not get_option('rga').disabled()
  subdir('rga')
endif
