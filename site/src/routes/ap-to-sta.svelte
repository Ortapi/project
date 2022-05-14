<script lang="ts">
  import { onMount } from "svelte";
  let aps = [];
  let ssid = "";
  let password = "";
  // const getAps = async () => {
  //   const res = await fetch("/api/get-ap-list");
  //   aps = await res.json();
  //   console.log(aps);
  // };
  //onMount(getAps);
  const joinNetwork = async () => {
    try {
      const result = await fetch("/api/ap-sta", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ ssid, password }),
      });
      if (result.ok) {
        console.log(result);
        alert("network changed");
        document.location.href = "/";
      }
    } catch (err) {
      alert(err);
    }
  };
</script>

<div class="bg-indigo-300 object-cover flex-col text-center ">

  <h1 class="text-xl" >WIFI Config</h1>

</div>

  <div class="bg-indigo-300 object-cover flex-col text-center ">
      <input bind:value={ssid}  type="text" class="input input-bordered w-full max-w-xs text-center">
  </div>
<!-- <div class="bg-indigo-300 object-cover flex-col text-center "> -->

<div class="bg-indigo-300 object-cover flex-col text-center ">
  
  <input bind:value={password} type="password" placeholder="password" class="input input-bordered w-full max-w-xs text-center" />
</div>
  <div class="bg-indigo-300 object-cover flex-col text-center ">
  <button on:click={joinNetwork} class="btn m-3 flex-col text-center">Submit</button>
</div>
<!-- </div> -->


<!-- {ssid}
{password} -->