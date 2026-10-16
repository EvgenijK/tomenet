#!/usr/bin/env python3
"""Extract production gate/helpers/config branch, compile, and verify without a server.

The harness stubs candidate eligibility and RNG only; it tests attempt probabilities,
not successful placement or full-floor RNG sequences. Run from any directory.
"""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
generator = (ROOT / 'src/server/generate.c').read_text()
config = (ROOT / 'src/server/init2.c').read_text()

def balanced(text, start):
    opening = text.index('{', start)
    level = 1
    end = opening + 1
    while level:
        level += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]

names = ['room_template_multiplier', 'room_template_total_weight',
         'room_template_random', 'room_template_category_roll', 'room_template_pick']
helpers = '\n'.join(balanced(generator, re.search(r'static [^\n]+\b' + name + r'\(', generator).start()) for name in names)
start = generator.index('if (room_template_weight &&\n')
end = generator.index(') {', start)
gate = generator[start + len('if ('):end]
assert 'DUN_UNUSUAL' not in gate
assert gate.count('rand_int(DUN_ROOM_TEMPLATE)') == 2
assert generator.index('if (destroyed)', generator.index('/* No rooms yet */')) < start
assert generator.index('if (d_info[d_ptr->type].room_templates_always', generator.index('/* No rooms yet */')) < start
assert start < generator.index('/* Attempt an "unusual" room */', start)
body = balanced(generator, start)
assert 'build_room_template' in body and body.rstrip().endswith('continue;\n\t\t\t}')
unusual = balanced(generator, generator.index('if (rand_int(DUN_UNUSUAL)', start))
assert 'room_template_' not in unusual
parser = balanced(config, config.index('else if (!strcmp(option, "DUN_ROOM_TEMPLATE"))'))
parser = parser[parser.index('{'):]
prefix = r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <errno.h>
typedef uint64_t u64b; typedef uint32_t u32b; typedef int32_t s32b; typedef int16_t s16b; typedef int bool;
#define TRUE 1
#define FALSE 0
#define ROOM_TEMPLATE_LIST_ALLOW 1
struct { int dun_unusual; s16b dun_room_template; } cfg = {120,120};
#define DUN_ROOM_TEMPLATE (cfg.dun_room_template)
#define DUN_UNUSUAL (cfg.dun_unusual)
typedef struct {u32b weight;} room_template_type;
typedef struct {int mode; bool *members; u32b *multipliers;} room_template_dungeon_rule;
static room_template_type room_templates[2]={{100},{300}};
static size_t room_template_count=2; static bool members[2]={1,1}; static u32b multipliers[2]={1000,1000};
static room_template_dungeon_rule room_template_dungeon_rules[1]={{1,members,multipliers}};
static unsigned long rng_calls; static u64b state=17; static int scripted, script[2], cursor, diagnostic_calls;
static s32b rand_int(s32b n) {
 assert(n>0); rng_calls++;
 if(scripted){assert(cursor<2); assert(script[cursor]<n); return script[cursor++];}
 state^=state<<13;state^=state>>7;state^=state<<17;return state%(u32b)n;
}
static void s_printf(const char *format,...) {(void)format;diagnostic_calls++;}
static bool room_template_is_eligible(size_t i,int real,int effective,int depth) {(void)real;(void)effective;(void)depth;return i<2;}
'''
tests = r'''
static void exact_depth(int threshold,int depth) {
 cfg.dun_room_template=threshold;
 int accepted=0, clipped=depth<0?0:depth>threshold?threshold:depth;
 for(int a=0;a<threshold;a++) for(int b=0;b<threshold;b++) {
  script[0]=a;script[1]=b;cursor=0;scripted=1;
  accepted+=template_gate(depth,4400000,400); /* saturated category: only depth draws */
 }
 assert(accepted==clipped*clipped);
 printf("EXACT_DEPTH_PASS threshold %d depth %d outcomes %d/%d\n",threshold,depth,accepted,threshold*threshold);
 scripted=0;
}
static void distribution(int depth,u32b a,u32b b) {
 cfg.dun_room_template=120;multipliers[0]=a;multipliers[1]=b;
 u64b baseline,scaled=room_template_total_weight(0,0,depth,&baseline);
 assert(baseline==400);assert(scaled==(u64b)100*a+(u64b)300*b);
 const int n=500000;int first=0, attempts=0;
 double depth_probability=fmin((double)depth/120,1);depth_probability*=depth_probability;
 double category=fmin((double)scaled/(baseline*11000),1);
 double expected=depth_probability*category;
 double share=scaled?(double)100*a/scaled:0;
 state=17;
 for(int i=0;i<n;i++) if(template_gate(depth,scaled,baseline)) {
  room_template_type *r=room_template_pick(0,0,depth,scaled); assert(r);
  attempts++;first+=(r==room_templates);
 }
 double measured=(double)attempts/n;
 assert(fabs(measured-expected)<0.0015);
 if(attempts) assert(fabs((double)first/attempts-share)<0.02);
 printf("DISTRIBUTION_PASS depth %d multipliers %g/%g attempt %.6f expected %.6f first_share %.6f expected %.6f\n",depth,a/1000.0,b/1000.0,measured,expected,attempts?(double)first/attempts:0,share);
}
int main(void) {
 exact_depth(1,0);exact_depth(1,1);exact_depth(7,3);exact_depth(120,0);
 exact_depth(120,1);exact_depth(120,30);exact_depth(120,60);exact_depth(120,119);
 exact_depth(120,120);exact_depth(120,121);
 cfg.dun_room_template=120;
 int unusual_values[]={1,120,32767};
 u64b reference_state=0;int reference_count=0;
 for(int u=0;u<3;u++) {
  cfg.dun_unusual=unusual_values[u];state=19;int hits=0;
  for(int i=0;i<50000;i++) hits+=template_gate(60,400000,400);
  if(!u){reference_state=state;reference_count=hits;}
  assert(state==reference_state && hits==reference_count);
 }
 puts("UNUSUAL_PARAMETER_INDEPENDENCE_PASS (same isolated-gate RNG stream)");
 unsigned long before=rng_calls;
 assert(!template_gate(120,0,400)); assert(before==rng_calls);
 assert(!room_template_category_roll(0,400));assert(!room_template_category_roll(1,0));
 assert(!room_template_pick(0,0,120,0)); assert(before==rng_calls);
 puts("ZERO_WEIGHT_NO_RNG_PASS");
 distribution(30,1000,1000);distribution(60,1000,1000);distribution(120,1000,1000);
 distribution(60,2000,1000);distribution(120,2000,1000);distribution(120,0,1000);
 distribution(120,0,0);distribution(120,11000,11000);distribution(120,1000000,1000);
 const char *valid[]={"1","120","32767"};int expected[]={1,120,32767};
 for(int i=0;i<3;i++){cfg.dun_room_template=77;parse_template(valid[i]);assert(cfg.dun_room_template==expected[i]);}
 const char *invalid[]={"0","-1","32768","","abc","12tail","1.5","999999999999999999999999999999999999999"};
 for(unsigned i=0;i<sizeof(invalid)/sizeof(invalid[0]);i++){
  cfg.dun_room_template=77;int before=diagnostic_calls;parse_template(invalid[i]);
  assert(cfg.dun_room_template==77);assert(diagnostic_calls==before+1);
 }
 puts("CONFIG_RANGE_PARSE_AND_RETAIN_PASS (3 accepted, 8 rejected)");
 puts("INDEPENDENT_TEMPLATE_CHECK_PASS");
}
'''
source = prefix + helpers + '\nstatic bool template_gate(int dun_lev, u64b room_template_weight, u64b room_template_baseline) { return (' + gate + '); }\nstatic void parse_template(const char *value) ' + parser + tests
with tempfile.TemporaryDirectory(prefix='independent-template-') as temp:
    temp = Path(temp)
    (temp/'harness.c').write_text(source)
    subprocess.run(['cc','-std=c99','-O1','-Wall','-Wextra','-Werror','-fsanitize=undefined','-fno-sanitize-recover=all',str(temp/'harness.c'),'-lm','-o',str(temp/'harness')], check=True)
    subprocess.run([str(temp/'harness')], check=True)
print('SOURCE_PLACEMENT_AND_ATTEMPT_CONSUMPTION_PASS')
