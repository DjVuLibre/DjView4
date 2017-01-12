
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "libdjvu/ddjvuapi.h"


/* handle ddjvu messages */

static void
handle(ddjvu_context_t *ctx, int wait)
{
  const ddjvu_message_t *msg;
  if (!ctx)
    return;
  if (wait)
    msg = ddjvu_message_wait(ctx);
  while ((msg = ddjvu_message_peek(ctx)))
    {
      switch(msg->m_any.tag)
        {
	case DDJVU_ERROR:
	  NSLog(@"%s", msg->m_error.message);
	  if (msg->m_error.filename)
	    NSLog(@"'%s:%d'",
		  msg->m_error.filename, msg->m_error.lineno);
	default:
	  break;
        }
      ddjvu_message_pop(ctx);
    }
}

/* -----------------------------------------------------------------------------
    Generate a thumbnail for file

   This function's job is to create thumbnail for designated file as fast as possible
   ----------------------------------------------------------------------------- */

OSStatus
GenerateThumbnailForURL(void *thisInterface,
                        QLThumbnailRequestRef thumbnail,
                        CFURLRef url,
                        CFStringRef contentTypeUTI,
                        CFDictionaryRef options,
                        CGSize maxSize)
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSDictionary *domain = [defaults persistentDomainForName:@"org.djvu.qlgenerator"];
  NSString *path = (NSString *)CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
  ddjvu_context_t *ctx = 0;
  ddjvu_document_t *doc = 0;
  ddjvu_format_t *fmt = 0;
  int width, height;
  NSBitmapImageRep *bitmap;
  unsigned char *bytes = 0;

  /* If not generating thumbnails */
  if (domain && [domain objectForKey:@"thumbnails"])
    if ([[domain objectForKey:@"thumbnails"] boolValue] == FALSE)
      goto pop;
  
  /* Create context and document */
  if (! (ctx = ddjvu_context_create([[[NSProcessInfo processInfo] processName]
				      cStringUsingEncoding:NSASCIIStringEncoding]))) {
    NSLog(@"Cannot create djvu context for '%@'.", path);
    goto pop;
  }
  if (! (doc = ddjvu_document_create_by_filename(ctx, [path fileSystemRepresentation], TRUE))) {
    NSLog(@"Cannot open djvu document '%@'.", path);
    goto pop;
  }
  while (! ddjvu_document_decoding_done(doc))
    handle(ctx, TRUE);
  if (ddjvu_document_decoding_error(doc)) {
    NSLog(@"Djvu document decoding error '%@'.", path);
    goto pop;
  }
  
  /* Prepare thumbnail */
  while (ddjvu_thumbnail_status(doc, 0, 1) < DDJVU_JOB_OK)
    handle(ctx, TRUE);
  if (ddjvu_thumbnail_status(doc, 0, 0) != DDJVU_JOB_OK) {
    NSLog(@"Djvu thumbnail generation error '%@'.", path);
    goto pop;
  }
  
  /* Get thumbnail size */
  width = (int) maxSize.width;
  height = (int) maxSize.height;
  fmt = ddjvu_format_create(DDJVU_FORMAT_RGB24, 0, NULL);
  ddjvu_format_set_row_order(fmt, TRUE);
  if (!ddjvu_thumbnail_render(doc, 0, &width, &height, fmt, width*3, NULL)) {
    NSLog(@"Djvu thumbnail rendering error '%@'.", path);
    goto pop;
  }

  /* Get thumbnail image */
  bytes = (unsigned char*)malloc(width * height * 3);
  if (!bytes || !ddjvu_thumbnail_render(doc, 0, &width, &height, fmt, width*3, bytes)) {
    NSLog(@"Djvu thumbnail rendering error '%@'.", path);
    goto pop;
  }
  bitmap = [[NSBitmapImageRep alloc] autorelease];
  bitmap = [bitmap initWithBitmapDataPlanes:&bytes
				 pixelsWide:width
				 pixelsHigh:height
			      bitsPerSample:8
			    samplesPerPixel:3
				   hasAlpha:FALSE
				   isPlanar:NO
			     colorSpaceName:NSCalibratedRGBColorSpace
				bytesPerRow:width*3
			       bitsPerPixel:24 ];
  if (bitmap)
    QLThumbnailRequestSetImageWithData(thumbnail, (CFDataRef)[bitmap TIFFRepresentation], NULL);
  
  /* Cleanup */
 pop:
  if (bytes)
    free(bytes);
  if (fmt)
    ddjvu_format_release(fmt);
  if (doc)
    ddjvu_document_release(doc);
  if (ctx)
    ddjvu_context_release(ctx);
  [pool release];
  return noErr;
}

void CancelThumbnailGeneration(void* thisInterface, QLThumbnailRequestRef thumbnail)
{
    // implement only if supported
}
