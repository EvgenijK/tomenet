// Check the strict object-shape requirement locally before any inference.
// This is intentionally a compatibility guard, not a replacement for the API's
// complete JSON Schema validator or validation of an agent's response.
export function assertStrictOutputSchema(schema){
 const need=(ok)=>{if(!ok)throw new Error('Strict output schema requires closed objects with every property required');};
 const visit=s=>{
  need(s&&typeof s==='object'&&!Array.isArray(s));
  const object=s.type==='object'||Array.isArray(s.type)&&s.type.includes('object');
  if(object){
   const properties=s.properties??{},keys=Object.keys(properties),required=s.required;
   need(properties&&typeof properties==='object'&&!Array.isArray(properties)&&s.additionalProperties===false&&Array.isArray(required)&&required.every(k=>typeof k==='string')&&new Set(required).size===required.length&&required.length===keys.length&&keys.every(k=>required.includes(k)));
  }
  for(const key of ['properties','$defs','definitions'])if(s[key])for(const child of Object.values(s[key]))visit(child);
  for(const key of ['anyOf','oneOf','allOf'])if(s[key]){need(Array.isArray(s[key]));for(const child of s[key])visit(child);}
  if(s.items)for(const child of Array.isArray(s.items)?s.items:[s.items])visit(child);
 };
 visit(schema);return schema;
}
